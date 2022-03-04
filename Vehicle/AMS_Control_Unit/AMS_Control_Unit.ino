/* ACU CONTROL UNIT CODE
   The AMS Control Unit code is used to control and communicate with Analog Devices LTC6811-2 battery stack monitors, per the HyTech Racing HT06 Accumulator Design.
   It also handles CAN communications with the mainECU and energy meter, performs coulomb counting operations, and drives a watchdog timer on the ACU.
   See LTC6811_2.cpp and LTC6811-2 Datasheet provided by Analog Devices for more details.
   Author: Zekun Li, Liwei Sun
   Version: 0.2
   Since: 02/07/2022
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
#define MAX_SUCCESSIVE_FAULTS 3   // Number of successive faults permitted before AMS fault is broadcast over CAN 
#define MIN_VOLTAGE 30000          // Minimum allowable single cell voltage in units of 100μV
#define MAX_VOLTAGE 42000          // Maxiumum allowable single cell voltage in units of 100μV
#define MAX_TOTAL_VOLTAGE 3550000  // Maximum allowable pack total voltage in units of 100μV
#define MAX_THERMISTOR_VOLTAGE 26225   // Maximum allowable pack temperature corresponding to 60C in units 100μV
#define BALANCE_COOL 3             // Sets balancing duty cycle as 20%
#define BALANCE_STANDARD 8         // Sets balancing duty cycle as 53.3%
#define BALANCE_HOT 12             // Sets balancing duty cycle as 80%
#define BALANCE_CONTINUOUS 15      // Sets balancing duty cycle as 100% 

// VARIABLE DECLARATIONS
uint16_t pec15Table[256];          // Array containing lookup table for PEC generator
uint16_t* LTC6811_2::pec15Table_pointer = pec15Table;   // Pointer to the PEC lookup table
uint16_t vuv = 1874; // 3V           // Minimum voltage value following datasheet formula: Comparison Voltage = (VUV + 1) • 16 • 100μV
uint16_t vov = 2625; // 4.2V         // Maximum voltage value following datasheet formula: Comparison Voltage = VOV • 16 • 100μV
uint16_t cell_voltages[TOTAL_IC][12]; // 2D Array to hold cell voltages being read in; voltages are read in with the base unit as 100μV
uint32_t total_voltage;             // the total voltage of the pack
uint16_t balance_voltage = 65535;   // the voltage to balance toward with the base unit as 100μV; equal to the lowest voltage in the battery pack. Iniitalized to max 16 bit value in order to prevent balancing without having read in valid voltages first
int min_voltage_location[2]; // [0]: IC#; [1]: Cell#
int max_voltage_location[2]; // [0]: IC#; [1]: Cell#
uint16_t min_voltage = 65535;
uint16_t max_voltage = 0;
uint16_t gpio_voltages[TOTAL_IC][6];  // 2D Array to hold GPIO voltages being read in; voltages are read in with the base unit as 100μV
float gpio_temps[TOTAL_IC][6];      // 2D Array to hold GPIO temperatures being read in; temperatures are read in with the base unit as K
int max_temp_location[2];
int min_temp_location[2];
int max_thermistor_location[2];
int min_thermistor_location[2];
int max_humidity_location[2];
uint16_t max_humidity = 0;
uint16_t max_thermistor_voltage = 0;
uint16_t min_thermistor_voltage = 65535;
uint16_t max_temp_voltage = 0;
uint16_t min_temp_voltage = 65535;
Metro charging_timer = Metro(5000); // Timer to check if charger is still talking to ACU
IntervalTimer pulse_timer;    //ams ok pulse
bool next_pulse = true;

// CONSECUTIVE FAULT COUNTERS: counts successive faults; resets to zero if normal reading breaks fault chain
unsigned long uv_fault_counter = 0;             // undervoltage fault counter
unsigned long ov_fault_counter = 0;             // overvoltage fault counter
unsigned long pack_ov_fault_counter = 0;    // total voltage overvoltage fault counter
unsigned long overtemp_fault_counter = 0;    //total overtemperature fault counter
bool overtemp_fault_state = false; // enter fault state is 20 successive faults occur
bool uv_fault_state = false;      // enter fault state is 20 successive faults occur
bool ov_fault_state = false;      // enter fault state is 20 successive faults occur
bool pack_ov_fault_state = false; // enter fault state is 20 successive faults occur

// LTC6811_2 OBJECT DECLARATIONS
LTC6811_2 ic[8];

// CAN OBJECT AND VARIABLE DECLARATIONS
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN;
CAN_message_t msg;

// BMS CAN MESSAGE AND STATE MACHINE OBJECT DECLARATIONS
BMS_status bms_status; //Message class that contains flags for AMS errors as well as a variable encoding the current state of the AMS (charging vs. discharging)
BMS_voltages bms_voltages;
BMS_temperatures bms_temperatures;
BMS_onboard_temperatures bms_onboard_temperatures;
BMS_detailed_voltages bms_detailed_voltages;


void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(6, HIGH); //write Teensy_OK pin high
  pulse_timer.begin(ams_ok_pulse,50000); //timer to pulse pin 5 every 50 milliseconds
  Serial.begin(115200);
  SPI.begin();
  CAN.begin();
  CAN.setBaudRate(500000);
  
  for (int i = 0; i < 64; i++) { // Fill all filter slots with Charger Control Unit message filter (CAN controller requires filling all slots)
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
  print_voltages();
  print_gpios();
  if (bms_status.get_state() == BMS_STATE_CHARGING) {
    balance_cells(BALANCE_STANDARD);
  }
}

// READ functions to collect and read data from the LTC6811-2
// Read cell voltages from all eight LTC6811-2; voltages are read in with units of 100μV
void read_voltages() {
  total_voltage = 0;  
  Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x1F, false, false, vuv, vov, (uint16_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group
  for (int i = 0; i < 8; i++) {
    ic[i].wakeup();
    ic[i].wrcfga(configuration);
    uint8_t *wrfcga_buf = configuration.buf();
    Reg_Group_Config reg_group_config = ic[i].rdcfga();
    ic[i].adcv(static_cast<CELL_SELECT>(0), false);
  }
  delay(203);
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
  voltage_fault_check();
  bms_voltages.set_low(min_voltage);
  bms_voltages.set_high(max_voltage);
  bms_voltages.set_average(total_voltage / 84);
  bms_voltages.set_total(total_voltage >> 7);
  
}

void voltage_fault_check(){
    balance_voltage = min_voltage;
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
  double total_cell_temps = 0;
  double total_thermistor_temps = 0;
  Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x1F, false, false, vuv, vov, (uint16_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group
  for (int i = 0; i < 8; i++) {
    ic[i].wakeup();
    ic[i].wrcfga(configuration);
    ic[i].adax(static_cast<GPIO_SELECT>(0), false);
    }
  delay(203);
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
        if ((i % 2) && j+k ==4) {
          gpio_temps[i][j + k] = -66.875 + 218.75 * (gpio_voltages[i][j + k] / 50000.0); // caculation for SHT31 temperature in C
          total_cell_temps += gpio_temps[i][j+k];
          if (gpio_voltages[i][4] > max_temp_voltage) {
            max_temp_voltage = gpio_voltages[i][j + k];
            max_temp_location[0] = i;
            max_temp_location[1] = j + k;
          }
          if (gpio_voltages[i][4] > min_temp_voltage) {
            min_temp_voltage = gpio_voltages[i][j + k];
            min_temp_location[0] = i;
            min_temp_location[1] = j + k;
          }
        }else{
          gpio_temps[i][j + k] = -12.5 + 125*(gpio_voltages[i][j + k])/50000.0;
          float thermistor_resistance = (2740 / (gpio_voltages[i][j + k] / 50000.0)) - 2740;
          gpio_temps[i][j + k] = 1 / ((1 / 298.15) + (1 / 3984.0) * log(thermistor_resistance / 10000.0)) - 273.15; //calculation for thermistor temperature in C
          if (j+k <=3 && gpio_voltages[i][j+k] > max_thermistor_voltage) {
            max_thermistor_voltage = gpio_voltages[i][j + k];
            max_thermistor_location[0] = i;
            max_thermistor_location[1] = j + k;
          }
          if (j+k <=3 && gpio_voltages[i][j+k] < min_thermistor_voltage) {
            min_thermistor_voltage = gpio_voltages[i][j + k];
            min_thermistor_location[0] = i;
            min_thermistor_location[1] = j + k;
          }
          if (j+k == 4 && gpio_temps[i][j+k] > max_humidity) {
            max_humidity = gpio_temps[i][j + k];
            max_humidity_location[0] = i;
            max_humidity_location[1] = j + k;
          }
        }
      }
    }
  }
  void temp_fault_check();
  
  bms_temperatures.set_low_temperature((uint16_t) (gpio_temps[min_temp_location[0]][min_temp_location[1]] * 100));
  bms_temperatures.set_high_temperature((uint16_t) (gpio_temps[max_temp_location[0]][max_temp_location[1]] *100));
  bms_temperatures.set_average_temperature((uint16_t)(total_cell_temps * 100 / 32));

  bms_onboard_temperatures.set_low_temperature((uint16_t) gpio_temps[min_thermistor_location[0]][min_thermistor_location[1]] * 100);
  bms_onboard_temperatures.set_high_temperature((uint16_t) gpio_temps[min_thermistor_location[0]][min_thermistor_location[1]] * 100);
  bms_onboard_temperatures.set_average_temperature((uint16_t)(total_thermistor_temps / 32));
}

void temp_fault_check(){
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
void balance_cells(uint8_t mode) {
  static int i = 0; // counter to track which IC is balancing its cells
  uint16_t cell_balance_setting = 0x0;
  if (balance_voltage < 30000 || balance_voltage > 42000) {
    Serial.print("BALANCE HALT: BALANCE VOLTAGE SET AS "); Serial.print(balance_voltage / 10000.0, 4); Serial.println(", OUTSIDE OF SAFE BOUNDS.");
    return;
  }
  if (overtemp_fault_state || uv_fault_state || ov_fault_state || pack_ov_fault_state) {
    Serial.print("BALANCE HALT: CHECK PACK FAULTS");
  }
  // determine which cells of the IC need balancing
  for (int cell = 0; cell < 12; cell++) {
    if (cell_voltages[i][cell] - balance_voltage > 10) {
      cell_balance_setting = 0x1 << cell | cell_balance_setting;
    }
  }
  Serial.print("Balancing voltage: "); Serial.println(balance_voltage / 10000.0, 4);
  Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x1F, false, false, vuv, vov, (uint16_t) cell_balance_setting, (uint8_t) 0x1); // base configuration for the configuration register group
  if (i < 8) {
    Serial.print("Currently balancing cell #: "); Serial.println(i, DEC);
    ic[i].wakeup();
    ic[i].wrcfga(configuration);
    if (i == 7) {
      i = 0;
    } else {
      i++;
    }
  }
  delay(500);
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

void write_CAN_messages() {
  //Write BMS_status message
  msg.id = ID_BMS_STATUS;
  msg.len = sizeof(bms_status);
  bms_status.write(msg.buf);
//  for (int i = 0; i < 8; i++) {
//    Serial.println(msg.buf[i], BIN);
//  }
  CAN.write(msg);

  msg.id = ID_BMS_VOLTAGES;
  msg.len = sizeof(bms_voltages);
  bms_voltages.write(msg.buf);
  CAN.write(msg);

  msg.id = ID_BMS_TEMPERATURES;
  msg.len = sizeof(bms_temperatures);
  bms_temperatures.write(msg.buf);
  CAN.write(msg);

  msg.id = ID_BMS_ONBOARD_TEMPERATURES;
  msg.len = sizeof(bms_onboard_temperatures);
  bms_onboard_temperatures.write(msg.buf);
  CAN.write(msg);

  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 12; j+=3){
      if(!(i%2 && j==9))
        bms_detailed_voltages.set_ic_id(i);
        bms_detailed_voltages.set_group_id(j);
        bms_detailed_voltages.set_voltage_0(cell_voltages[i][j]);
        bms_detailed_voltages.set_voltage_1(cell_voltages[i][j+1]);
        bms_detailed_voltages.set_voltage_2(cell_voltages[i][j+2]);
        msg.id = ID_BMS_DETAILED_VOLTAGES;
        msg.len = sizeof(bms_detailed_voltages);
        bms_detailed_voltages.write(msg.buf);
        CAN.write(msg);
    }
  }
    
}

// Pulses pin 5 to keep watchdog circuit active
void ams_ok_pulse(){
  if(!overtemp_fault_state && !uv_fault_state && !ov_fault_state  && !pack_ov_fault_state){
    next_pulse = !next_pulse;
  }
    digitalWrite(5,(next_pulse?HIGH:LOW));
}

// Data print functions
// Print cell voltages
void print_voltages() {
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  if (min_voltage < MIN_VOLTAGE) {
    Serial.print("UNDERVOLTAGE FAULT: "); Serial.print("IC #: "); Serial.print(min_voltage_location[0]); Serial.print("\tCell #: "); Serial.print(min_voltage_location[1]); Serial.print("\tConsecutive fault #: "); Serial.println(uv_fault_counter);
  }
  if (max_voltage > MAX_VOLTAGE) {
    Serial.print("OVERVOLTAGE FAULT: "); Serial.print("IC #: "); Serial.print(max_voltage_location[0]); Serial.print("\tCell #: "); Serial.print(max_voltage_location[1]); Serial.print("\tConsecutive fault #: "); Serial.println(ov_fault_counter); 
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
  Serial.print("OVERTEMP FAULT: ");Serial.print("\tConsecutive fault #: "); Serial.println(overtemp_fault_counter);
  }
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
    }else {
      Serial.print("PCB Humidity: "); Serial.print(gpio_temps[ic][4], 3); Serial.print("%\t");
    }
    Serial.print("\t");
    Serial.println();
  }
  Serial.print("Max Board Temp: "); Serial.print(gpio_temps[max_temp_location[0]][max_temp_location[1]],3);Serial.print("C \t "); 
  Serial.print("Max Thermistor Temp: "); Serial.print(gpio_temps[max_thermistor_location[0]][max_thermistor_location[1]],3);Serial.print("C \t "); 
  Serial.print("Max Humidity: "); Serial.print(gpio_temps[max_humidity_location[0]][max_humidity_location[1]],3);Serial.println("% \t "); 
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  
  
}
