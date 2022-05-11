/* ACU CONTROL UNIT CODE
   The AMS Control Unit code is used to control and communicate with Analog Devices LTC6811-2 battery stack monitors, per the HyTech Racing HT06 Accumulator Design.
   It also handles CAN communications with the mainECU and energy meter, performs coulomb counting operations, and drives a watchdog timer on the ACU.
   See LTC6811_2.cpp and LTC6811-2 Datasheet provided by Analog Devices for more details.
   Author: Zekun Li, Liwei Sun
   Version: 1.02
   Since: 04/23/2022
*/

#include <Arduino.h>
#include <SPI.h>
#include <FlexCAN_T4.h>
#include <HyTech_CAN.h>
#include <LTC6811_2.h>
#include <Metro.h>

// CONSTANT DEFINITIONS: define important values, such as IC count and cells per IC
#define TOTAL_IC 8                 // Number of LTC6811-2 ICs that are used in the accumulator
#define EVEN_IC_CELLS 12           // Number of cells monitored by ICs with even addresses
#define ODD_IC_CELLS 9             // Number of cells monitored by ICS with odd addresses
#define THERMISTORS_PER_IC 4       // Number of cell temperature monitoring thermistors connected to each IC 
#define MAX_SUCCESSIVE_FAULTS 20   // Number of successive faults permitted before AMS fault is broadcast over CAN 
#define MIN_VOLTAGE 30000          // Minimum allowable single cell voltage in units of 100μV
#define MAX_VOLTAGE 42000          // Maxiumum allowable single cell voltage in units of 100μV
#define MAX_TOTAL_VOLTAGE 3550000  // Maximum allowable pack total voltage in units of 100μV
#define MAX_THERMISTOR_VOLTAGE 26225   // Maximum allowable pack temperature corresponding to 60C in units 100μV
#define BALANCE_ON false
#define BALANCE_COOL 6000             // Sets balancing duty cycle as 33.3%
#define BALANCE_STANDARD 4000         // Sets balancing duty cycle as 50%
#define BALANCE_HOT 3000             // Sets balancing duty cycle as 66%
#define BALANCE_CONTINUOUS 2000     // Sets balancing duty cycle as 100%

// VARIABLE DECLARATIONS
uint16_t pec15Table[256];          // Array containing lookup table for PEC generator
uint16_t* LTC6811_2::pec15Table_pointer = pec15Table;   // Pointer to the PEC lookup table
uint16_t vuv = 1874; // 3V           // Minimum voltage value following datasheet formula: Comparison Voltage = (VUV + 1) • 16 • 100μV
uint16_t vov = 2625; // 4.2V         // Maximum voltage value following datasheet formula: Comparison Voltage = VOV • 16 • 100μV
uint16_t cell_voltages[TOTAL_IC][12]; // 2D Array to hold cell voltages being read in; voltages are read in with the base unit as 100μV
uint32_t total_voltage;             // the total voltage of the pack
int min_voltage_location[2]; // [0]: IC#; [1]: Cell#
int max_voltage_location[2]; // [0]: IC#; [1]: Cell#
uint16_t min_voltage = 65535;
uint16_t max_voltage = 0;
uint16_t gpio_voltages[TOTAL_IC][6];  // 2D Array to hold GPIO voltages being read in; voltages are read in with the base unit as 100μV
float gpio_temps[TOTAL_IC][6];      // 2D Array to hold GPIO temperatures being read in; temperatures are read in with the base unit as K
int max_board_temp_location[2]; // [0]: IC#; [1]: Cell#
int min_board_temp_location[2]; // [0]: IC#; [1]: Cell#
int max_thermistor_location[2]; // [0]: IC#; [1]: Cell#
int max_humidity_location[2]; // [0]: IC#; [1]: Cell#
int min_thermistor_location[2]; // [0]: IC#; [1]: Cell#
uint16_t max_humidity = 0;
uint16_t max_thermistor_voltage = 0;
uint16_t min_thermistor_voltage = 65535;
uint16_t max_board_temp_voltage = 0;
uint16_t min_board_temp_voltage = 65535;
float total_board_temps = 0;
float total_thermistor_temps = 0;
Metro charging_timer = Metro(5000); // Timer to check if charger is still talking to ACU
Metro CAN_timer = Metro(2); // Timer that spaces apart writes for CAN messages so as to not saturate CAN bus
Metro print_timer = Metro(500);
Metro balance_timer(BALANCE_STANDARD);
elapsedMillis adc_timer; // timer that determines wait time for ADCs to finish their conversions
uint8_t adc_state; // 0: wait to begin voltage conversions; 1: adcs converting voltage values; 2: wait to begin gpio conversions; 3: adcs converting GPIO values
IntervalTimer pulse_timer;    //AMS ok pulse timer
bool next_pulse = true; //AMS ok pulse
int can_voltage_ic = 0; //counter for the current IC data to send for detailed voltage CAN message
int can_voltage_group = 0; // counter for current group data to send for detailed voltage CAN message
int can_gpio_ic = 0; //counter for the current IC data to send for detailed voltage CAN message
int can_gpio_group = 0; // counter for current group data to send for detailed voltage CAN message
elapsedMillis can_bms_status_timer = 0;
elapsedMillis can_bms_detailed_voltages_timer = 2;
elapsedMillis can_bms_detailed_temps_timer = 4;
elapsedMillis can_bms_voltages_timer = 6;
elapsedMillis can_bms_temps_timer = 8;
elapsedMillis can_bms_onboard_temps_timer = 10;

// CONSECUTIVE FAULT COUNTERS: counts successive faults; resets to zero if normal reading breaks fault chain
unsigned long uv_fault_counter = 0;             // undervoltage fault counter
unsigned long ov_fault_counter = 0;             // overvoltage fault counter
unsigned long pack_ov_fault_counter = 0;    // total voltage overvoltage fault counter
unsigned long overtemp_fault_counter = 0;    //total overtemperature fault counter
bool overtemp_fault_state = false; // enter fault state if 20 successive faults occur
bool uv_fault_state = false;      // enter fault state if 20 successive faults occur
bool ov_fault_state = false;      // enter fault state if 20 successive faults occur
bool pack_ov_fault_state = false; // enter fault state if 20 successive faults occur

// LTC6811_2 OBJECT DECLARATIONS
LTC6811_2 ic[8];

// CAN OBJECT AND VARIABLE DECLARATIONS
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN;
CAN_message_t msg;

// BMS CAN MESSAGE AND STATE MACHINE OBJECT DECLARATIONS
BMS_status bms_status; //Message class that contains flags for AMS errors as well as a variable encoding the current state of the AMS (charging vs. discharging)
BMS_voltages bms_voltages; //Message class containing general voltage information
BMS_temperatures bms_temperatures; //Message class containing general temperature information
BMS_onboard_temperatures bms_onboard_temperatures; //Message class containing general AMS temperature information
BMS_detailed_voltages bms_detailed_voltages; //Message class containing detailed voltage information
BMS_detailed_temperatures bms_detailed_temperatures; // message class containing detailed temperature information


void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(6, HIGH); //write Teensy_OK pin high
  pulse_timer.begin(ams_ok_pulse, 50000); //timer to pulse pin 5 every 50 milliseconds
  Serial.begin(115200);
  SPI.begin();
  CAN.begin();
  CAN.setBaudRate(500000);

  for (int i = 0; i < 64; i++) { // Fill all filter slots with Charger Control Unit message filter
    CAN.setMBFilter(static_cast<FLEXCAN_MAILBOX>(i), ID_CCU_STATUS); // Set CAN mailbox filtering to only watch for charger controller status CAN messages
  }
  // initialize the PEC table
  LTC6811_2::init_PEC15_Table();
  // add 8 instances of LTC6811_2 to the object array, each addressed appropriately
  for (int i = 0; i < 8; i++) {
    ic[i] = LTC6811_2(i);
  }
  /* Initialize ACU state machine
      The state machine has two states, dependent on whether or not the charger controller is talking to the ACU.
      The state machine initializes to the DISCHARGING state
  */
  bms_status.set_state(BMS_STATE_DISCHARGING);
  parse_CAN_CCU_status();

}
void loop() {
  // put your main code here, to run repeatedly:
  parse_CAN_CCU_status();
  if (charging_timer.check() && bms_status.get_state() == BMS_STATE_CHARGING) {
    bms_status.set_state(BMS_STATE_DISCHARGING);
  }
  read_voltages();
  read_gpio();
  write_CAN_messages();
  if (print_timer.check()) {
    print_voltages();
    print_gpios();
    print_timer.reset();
  }
  if (bms_status.get_state() == BMS_STATE_CHARGING && BALANCE_ON) {
    balance_cells();
  }
}

// READ functions to collect and read data from the LTC6811-2
// Read cell voltages from all eight LTC6811-2; voltages are read in with units of 100μV
void read_voltages() {
  if (adc_state == 0) {
    Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x1F, false, false, vuv, vov, (uint16_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group
    for (int i = 0; i < 8; i++) {
      ic[i].wakeup();
      ic[i].wrcfga(configuration);
      ic[i].adcv(static_cast<CELL_SELECT>(0), false);
    }
    adc_state = 1;
    adc_timer = 0;
  }
  if (adc_state == 1 && adc_timer > 203) {
    total_voltage = 0;
    max_voltage = 0;
    min_voltage = 65535;
    for (int i = 0; i < 8; i++) {
      ic[i].wakeup();
      Reg_Group_Cell_A reg_group_a = ic[i].rdcva();
      Reg_Group_Cell_B reg_group_b = ic[i].rdcvb();
      Reg_Group_Cell_C reg_group_c = ic[i].rdcvc();
      Reg_Group_Cell_D reg_group_d = ic[i].rdcvd();
      for (int j = 0; j < 12; j += 3) {
        uint8_t *buf;
        if (j == 0) {
          buf = reg_group_a.buf();
        } else if (j == 3) {
          buf = reg_group_b.buf();
        } else if (j == 6) {
          buf = reg_group_c.buf();
        } else if (j == 9) {
          if (i % 2 == 0) {
            buf = reg_group_d.buf();
          } else {
            break;
          }
        }
        for (int k = 0; k < 3; k++) {
          cell_voltages[i][j + k] = buf[2 * k + 1] << 8 | buf[2 * k];
          total_voltage += cell_voltages[i][j + k];
          if (cell_voltages[i][j + k] < min_voltage) {
            min_voltage = cell_voltages[i][j + k];
            min_voltage_location[0] = i;
            min_voltage_location[1] = j + k;
          }
          if (cell_voltages[i][j + k] > max_voltage) {
            max_voltage = cell_voltages[i][j + k];
            max_voltage_location[0] = i;
            max_voltage_location[1] = j + k;
          }
        }
      }
    }
    min_voltage = min_voltage;
    voltage_fault_check();
    adc_state = 2;
  }
}

void voltage_fault_check() {
  // detect any uv fault conditions, set appropriate error flags, and print relevant message to console
  if (min_voltage < MIN_VOLTAGE) {
    uv_fault_counter++;
  } else {
    uv_fault_counter = 0;
  }
  if (uv_fault_counter > MAX_SUCCESSIVE_FAULTS) {
    uv_fault_state = true;
  }
  if (uv_fault_state) {
    bms_status.set_undervoltage(true);
  }
  else {
    bms_status.set_undervoltage(false);
  }
  // detect any ov fault conditions, set appropriate error flags, and print relevant message to console
  if (max_voltage > MAX_VOLTAGE) {
    ov_fault_counter++;
  } else {
    ov_fault_counter = 0;
  }
  if (ov_fault_counter > MAX_SUCCESSIVE_FAULTS) {
    ov_fault_state = true;
  }
  if (ov_fault_state) {
    bms_status.set_overvoltage(true);
  } else {
    bms_status.set_overvoltage(false);
  }
  // detect any pack ov fault conditions, set appropriate error flags, and print relevant message to console
  if (total_voltage > MAX_TOTAL_VOLTAGE) {
    pack_ov_fault_counter++;
  } else {
    pack_ov_fault_counter = 0;
  }
  if (pack_ov_fault_counter > MAX_SUCCESSIVE_FAULTS) {
    pack_ov_fault_state = true;
  }
  if (pack_ov_fault_state) {
    bms_status.set_total_voltage_high(true);
  } else {
    bms_status.set_total_voltage_high(false);
  }
}

// Read GPIO registers from LTC6811-2; Process temperature and humidity data from relevant GPIO registers
void read_gpio() {
  if (adc_state == 2) {
    Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x1F, false, false, vuv, vov, (uint16_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group
    for (int i = 0; i < 8; i++) {
      ic[i].wakeup();
      ic[i].wrcfga(configuration);
      ic[i].adax(static_cast<GPIO_SELECT>(0), false);
    }
    adc_state = 3;
    adc_timer = 0;
  }
  if (adc_state == 3 && adc_timer > 203) {
    max_humidity = 0;
    max_thermistor_voltage = 0;
    min_thermistor_voltage = 65535;
    max_board_temp_voltage = 0;
    min_board_temp_voltage = 65535;
    total_board_temps = 0;
    total_thermistor_temps = 0;
    for (int i = 0; i < 8; i++) {
      ic[i].wakeup();
      Reg_Group_Aux_A reg_group_a = ic[i].rdauxa();
      Reg_Group_Aux_B reg_group_b = ic[i].rdauxb();
      for (int j = 0; j < 6; j += 3) {
        uint8_t *buf;
        if (j == 0) {
          buf = reg_group_a.buf();
        } else if (j == 3) {
          buf = reg_group_b.buf();
        }
        for (int k = 0; k < 3; k++) {
          gpio_voltages[i][j + k] = buf[2 * k + 1] << 8 | buf[2 * k];
          if ((i % 2) && j + k == 4) {
            gpio_temps[i][j + k] = -66.875 + 218.75 * (gpio_voltages[i][j + k] / 50000.0); // caculation for SHT31 temperature in C
            total_board_temps += gpio_temps[i][j + k];
            if (gpio_voltages[i][4] > max_board_temp_voltage) {
              max_board_temp_voltage = gpio_voltages[i][j + k];
              max_board_temp_location[0] = i;
              max_board_temp_location[1] = j + k;
            }
            if (gpio_voltages[i][4] < min_board_temp_voltage) {
              min_board_temp_voltage = gpio_voltages[i][j + k];
              min_board_temp_location[0] = i;
              min_board_temp_location[1] = j + k;
            }
          } else if (j + k == 4) {
            gpio_temps[i][j + k] = -12.5 + 125 * (gpio_voltages[i][j + k]) / 50000.0; // humidity calculation
            if (gpio_temps[i][j + k] > max_humidity) {
              max_humidity = gpio_temps[i][j + k];
              max_humidity_location[0] = i;
              max_humidity_location[1] = j + k;
            }
          } else if (j + k < 4) {
            float thermistor_resistance = (2740 / (gpio_voltages[i][j + k] / 50000.0)) - 2740;
            gpio_temps[i][j + k] = 1 / ((1 / 298.15) + (1 / 3984.0) * log(thermistor_resistance / 10000.0)) - 273.15; //calculation for thermistor temperature in C
            total_thermistor_temps += gpio_temps[i][j + k];
            if (gpio_voltages[i][j + k] > max_thermistor_voltage) {
              max_thermistor_voltage = gpio_voltages[i][j + k];
              max_thermistor_location[0] = i;
              max_thermistor_location[1] = j + k;
            }
            if (gpio_voltages[i][j + k] < min_thermistor_voltage) {
              min_thermistor_voltage = gpio_voltages[i][j + k];
              min_thermistor_location[0] = i;
              min_thermistor_location[1] = j + k;
            }

          }
        }
      }
    }
    temp_fault_check();
    adc_state = 0;
  }
}


void temp_fault_check() {
  if (max_thermistor_voltage > MAX_THERMISTOR_VOLTAGE) {
    overtemp_fault_counter++;
  } else {
    overtemp_fault_counter = 0;
  }
  if (overtemp_fault_counter > MAX_SUCCESSIVE_FAULTS) {
    overtemp_fault_state = true;
  }
  if (overtemp_fault_state) {
    bms_status.set_discharge_overtemp(true);
  } else {
    bms_status.set_discharge_overtemp(false);
  }
}

// Cell Balancing function. NOTE: Must call read_voltages() in order to obtain balancing voltage;
void balance_cells() {
  uint16_t cell_balance_setting = 0x0;
  if (balance_timer.check()) {
    balance_timer.reset();
    if (min_voltage < 30000 || min_voltage > 42000) {
      Serial.print("BALANCE HALT: BALANCE VOLTAGE SET AS "); Serial.print(min_voltage / 10000.0, 4); Serial.println(", OUTSIDE OF SAFE BOUNDS.");
      return;
    }
    if (overtemp_fault_state || uv_fault_state || ov_fault_state || pack_ov_fault_state) {
      Serial.print("BALANCE HALT: CHECK PACK FAULTS");
    }
    Serial.print("Balancing voltage: "); Serial.println(min_voltage / 10000.0, 4);
    for (uint16_t i = 0; i < 8; i++) {
      // determine which cells of the IC need balancing
      for (uint16_t cell = 0; cell < 12; cell++) {
        if (cell_voltages[i][cell] - min_voltage > 100) { // balance if the cell voltage differential from the minimum voltage is 0.01V or greater
          cell_balance_setting = (0b1 << cell) | cell_balance_setting;
        }
      }
      Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x1F, false, false, vuv, vov, (uint16_t) cell_balance_setting, (uint8_t) 0x0); // base configuration for the configuration register group
      ic[i].wakeup();
      ic[i].wrcfga(configuration);
    }
    delay(2000);
  }
}

// parse incoming CAN messages for CCU status message and changes the state of the BMS in software
void parse_CAN_CCU_status() {
  while (CAN.read(msg)) {
    if (msg.id == ID_CCU_STATUS) {
      charging_timer.reset();
      if (bms_status.get_state() == BMS_STATE_DISCHARGING) {
        bms_status.set_state(BMS_STATE_CHARGING);
      }
    }
  }
}

//CAN message write handler
void write_CAN_messages() {
  // set voltage message values
  bms_voltages.set_low(min_voltage);
  bms_voltages.set_high(max_voltage);
  bms_voltages.set_average(total_voltage / 84);
  bms_voltages.set_total(total_voltage / 100);
  // set temperature message values
  bms_temperatures.set_low_temperature(gpio_temps[min_thermistor_location[0]][min_thermistor_location[1]] * 100);
  bms_temperatures.set_high_temperature(gpio_temps[max_thermistor_location[0]][max_thermistor_location[1]] * 100);
  bms_temperatures.set_average_temperature(total_thermistor_temps * 100 / 32);
  // set onboard temperature message values
  bms_onboard_temperatures.set_low_temperature(gpio_temps[min_board_temp_location[0]][min_board_temp_location[1]] * 100);
  bms_onboard_temperatures.set_high_temperature(gpio_temps[max_board_temp_location[0]][max_board_temp_location[1]] * 100);
  bms_onboard_temperatures.set_average_temperature(total_board_temps * 100 / 4);

  //Write BMS_status message
  if (can_bms_status_timer > 100) {
    msg.id = ID_BMS_STATUS;
    msg.len = sizeof(bms_status);
    bms_status.write(msg.buf);
    CAN.write(msg);
    can_bms_status_timer = 0;
  }
  // Write BMS_voltages message
  if (can_bms_voltages_timer > 5) {
    msg.id = ID_BMS_VOLTAGES;
    msg.len = sizeof(bms_voltages);
    bms_voltages.write(msg.buf);
    CAN.write(msg);
    can_bms_voltages_timer = 0;
  }
  // Write BMS_temperatures message
  if (can_bms_temps_timer > 25) {
    msg.id = ID_BMS_TEMPERATURES;
    msg.len = sizeof(bms_temperatures);
    bms_temperatures.write(msg.buf);
    CAN.write(msg);
    can_bms_temps_timer = 0;
  }
  // Write BMS_onboard_temperatures message
  if (can_bms_onboard_temps_timer > 50) {
    msg.id = ID_BMS_ONBOARD_TEMPERATURES;
    msg.len = sizeof(bms_onboard_temperatures);
    bms_onboard_temperatures.write(msg.buf);
    CAN.write(msg);
    can_bms_onboard_temps_timer = 0;
  }
  // write detailed voltages for one IC group
  if (can_bms_detailed_voltages_timer > 10) {
    write_CAN_detailed_voltages();
    can_bms_detailed_voltages_timer = 0;
  }
  if (can_bms_detailed_temps_timer > 40) {
    write_CAN_detailed_temps();
    can_bms_detailed_temps_timer = 0;
  }
}

//detailed voltages CAN message handler; writes the CAN message for one ic group at a time
void write_CAN_detailed_voltages() {
  if (can_voltage_group > 9) {
    can_voltage_ic++;
    can_voltage_group = 0;
  }
  if (can_voltage_ic > 7) {
    can_voltage_ic = 0;
    can_voltage_group = 0;
  }
  if (!(can_voltage_ic % 2 && can_voltage_group == 9)) {
    bms_detailed_voltages.set_ic_id(can_voltage_ic);
    bms_detailed_voltages.set_group_id(can_voltage_group / 3);
    bms_detailed_voltages.set_voltage_0(cell_voltages[can_voltage_ic][can_voltage_group]);
    bms_detailed_voltages.set_voltage_1(cell_voltages[can_voltage_ic][can_voltage_group + 1]);
    bms_detailed_voltages.set_voltage_2(cell_voltages[can_voltage_ic][can_voltage_group + 2]);
    msg.id = ID_BMS_DETAILED_VOLTAGES;
    msg.len = sizeof(bms_detailed_voltages);
    bms_detailed_voltages.write(msg.buf);
    CAN.write(msg);
  }
  can_voltage_group += 3;
}

// TODO: This CAN message is in the HT05 Style; it needs to be updated with group ID to conform to HT06 standards
void write_CAN_detailed_temps() {
  if (can_gpio_group > 6) {
    can_gpio_ic++;
    can_gpio_group = 0;
  }
  if (can_gpio_ic > 7) {
    can_gpio_ic = 0;
  }
  bms_detailed_temperatures.set_ic_id(can_gpio_ic);
  bms_detailed_temperatures.set_group_id(can_gpio_group);
  bms_detailed_temperatures.set_temperature_0(gpio_temps[can_gpio_ic][can_gpio_group] * 100);
  bms_detailed_temperatures.set_temperature_1(gpio_temps[can_gpio_ic][can_gpio_group + 1] * 100);
  bms_detailed_temperatures.set_temperature_2(gpio_temps[can_gpio_ic][can_gpio_group + 2] * 100);
  msg.id = ID_BMS_DETAILED_TEMPERATURES;
  msg.len = sizeof(bms_detailed_temperatures);
  bms_detailed_temperatures.write(msg.buf);
  CAN.write(msg);
  can_gpio_group += 3;
}

// Pulses pin 5 to keep watchdog circuit active
void ams_ok_pulse() {
  if (!overtemp_fault_state && !uv_fault_state && !ov_fault_state  && !pack_ov_fault_state) {
    next_pulse = !next_pulse;
  }
  digitalWrite(5, (next_pulse ? HIGH : LOW));
}

// Data print functions
// Print cell voltages
void print_voltages() {
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  if (min_voltage < MIN_VOLTAGE) {
    Serial.print("UNDERVOLTAGE FAULT: "); Serial.print("IC #: "); Serial.print(min_voltage_location[0]); Serial.print("\tCell #: "); Serial.print(min_voltage_location[1]); Serial.print("\tFault Voltage: "); Serial.print(min_voltage / 10000.0, 4); Serial.print("\tConsecutive fault #: "); Serial.println(uv_fault_counter);
  }
  if (max_voltage > MAX_VOLTAGE) {
    Serial.print("OVERVOLTAGE FAULT: "); Serial.print("IC #: "); Serial.print(max_voltage_location[0]); Serial.print("\tCell #: "); Serial.print(max_voltage_location[1]); Serial.print("\tFault Voltage: "); Serial.print(max_voltage / 10000.0, 4);  Serial.print("\tConsecutive fault #: "); Serial.println(ov_fault_counter);
  }
  if (total_voltage > MAX_TOTAL_VOLTAGE) {
    Serial.print("PACK OVERVOLTAGE:"); Serial.print("\tConsecutive fault #: "); Serial.println(pack_ov_fault_counter);
  }

  Serial.print("Total pack voltage: "); Serial.print(total_voltage / 10000.0, 4); Serial.print("V\t"); Serial.print("Max voltage differential: "); Serial.print(max_voltage / 10000.0 - min_voltage / 10000.0, 4); Serial.println("V");
  Serial.print("AMS status: ");
  if (bms_status.get_state() == BMS_STATE_DISCHARGING) {
    Serial.println("Discharging");
  } else if (bms_status.get_state() == BMS_STATE_CHARGING) {
    Serial.println("Charging");
  }
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.print("Max Voltage: "); Serial.print(cell_voltages[max_voltage_location[0]][max_voltage_location[1]] / 10000.0, 4); Serial.print("V \t ");
  Serial.print("Min Voltage: "); Serial.print(cell_voltages[min_voltage_location[0]][min_voltage_location[1]] / 10000.0, 4); Serial.print("V \t");
  Serial.print("Avg Voltage: "); Serial.print(total_voltage / 840000.0, 4); Serial.println("V \t");
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.println("Raw Cell Voltages\t\t\t\t\t\t\tCell Status (Ignoring or Balancing)");
  Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8\tC9\tC10\tC11");
  for (int ic = 0; ic < TOTAL_IC; ic++) {
    Serial.print("IC"); Serial.print(ic); Serial.print("\t");
    for (int cell = 0; cell < EVEN_IC_CELLS; cell++) {
      Serial.print(cell_voltages[ic][cell] / 10000.0, 4); Serial.print("V\t");
    }
    Serial.print("\t");
    Serial.println();
  }
}

// Print values of temperature and humidity sensors in GPIOs
void print_gpios() {
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  if (max_thermistor_voltage > MAX_THERMISTOR_VOLTAGE) {
    Serial.print("OVERTEMP FAULT: "); Serial.print("\tConsecutive fault #: "); Serial.println(overtemp_fault_counter);
  }
  Serial.print("Max Board Temp: "); Serial.print(gpio_temps[max_board_temp_location[0]][max_board_temp_location[1]], 3); Serial.print("C \t ");
  Serial.print("Min Board Temp: "); Serial.print(gpio_temps[min_board_temp_location[0]][min_board_temp_location[1]], 3); Serial.print("C \t");
  Serial.print("Avg Board Temp: "); Serial.print(total_board_temps / 4, 3); Serial.println("C \t");
  Serial.print("Max Thermistor Temp: "); Serial.print(gpio_temps[max_thermistor_location[0]][max_thermistor_location[1]], 3); Serial.print("C \t");
  Serial.print("Min Thermistor Temp: "); Serial.print(gpio_temps[min_thermistor_location[0]][min_thermistor_location[1]], 3); Serial.print("C \t");
  Serial.print("Avg Thermistor Temp: "); Serial.print(total_thermistor_temps / 32, 3); Serial.println("C \t");
  Serial.print("Max Humidity: "); Serial.print(gpio_temps[max_humidity_location[0]][max_humidity_location[1]], 3); Serial.println("% \t ");
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.println("Raw Segment Temperatures");
  Serial.println("                  \tT0\tT1\tT2\tT3");
  for (int ic = 0; ic < TOTAL_IC; ic++) {
    Serial.print("Cell Temperatures"); Serial.print(ic); Serial.print("\t");
    for (int cell = 0; cell < 4; cell++) {
      Serial.print(gpio_temps[ic][cell], 3); Serial.print("C\t");
    }
    if ((ic % 2)) {
      Serial.print("PCB Temps: "); Serial.print(gpio_temps[ic][4], 3); Serial.print("C\t");
    } else {
      Serial.print("PCB Humidity: "); Serial.print(gpio_temps[ic][4], 3); Serial.print("%\t");
    }
    Serial.print("\t");
    Serial.println();
  }
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
}
