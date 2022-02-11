/* ACU CONTROL UNIT CODE
   The AMS Control Unit code is used to control and communicate with Analog Devices LTC6811-2 battery stack monitors, per the HyTech Racing HT06 Accumulator Design.
   It also handles CAN communications with the mainECU and energy meter, performs coulomb counting operations, and drives a watchdog timer on the ACU.
   See LTC6811_2.cpp and LTC6811-2 Datasheet provided by Analog Devices for more details.
   Author: Zekun Li
   Version: 0.1
   Since: 02/07/2022
*/

#include <Arduino.h>
#include <SPI.h>
#include <FlexCAN_T4.h>
#include <HyTech_CAN.h>
#include <LTC6811_2.h>
//#include <Metro.h>

// CONSTANT DEFINITIONS: define important values, such as IC count and cells per IC
#define TOTAL_IC 8                 // Number of LTC6811-2 ICs that are used in the accumulator
#define EVEN_IC_CELLS 12           // Number of cells monitored by ICs with even addresses
#define ODD_IC_CELLS 9             // Number of cells monitored by ICS with odd addresses
#define THERMISTORS_PER_IC 4       // Number of cell temperature monitoring thermistors connected to each IC 
#define MAX_SUCCESSIVE_FAULTS 20   // Number of successive faults permitted before AMS fault is broadcast over CAN 
#define MIN_VOLTAGE 30000          // Minimum allowable single cell voltage in units of 100μV
#define MAX_VOLTAGE 42000          // Maxiumum allowable single cell voltage in units of 100μV
#define MAX_TOTAL_VOLTAGE 3550000  // Maximum allowable pack total voltage in units of 100μV

// VARIABLE DECLARATIONS
uint16_t pec15Table[256];          // Array containing lookup table for PEC generator
uint16_t* LTC6811_2::pec15Table_pointer = pec15Table;   // Pointer to the PEC lookup table
uint16_t vuv = 1874; // 3V           // Minimum voltage value following datasheet formula: Comparison Voltage = (VUV + 1) • 16 • 100μV
uint16_t vov = 2625; // 4.2V         // Maximum voltage value following datasheet formula: Comparison Voltage = VOV • 16 • 100μV
uint16_t cell_voltages[TOTAL_IC][12]; // 2D Array to hold cell voltages being read in; voltages are read in with the base unit as 100μV
uint32_t total_voltage;             // the total voltage of the pack
uint16_t gpio_voltages[TOTAL_IC][6];  // 2D Array to hold GPIO voltages being read in; voltages are read in with the base unit as 100μV

// CONSECUTIVE FAULT COUNTERS: counts successive faults; resets to zero if normal reading breaks fault chain
int uv_fault_counter = 0;             // undervoltage fault counter
int ov_fault_counter = 0;             // overvoltage fault counter
int pack_ov_fault_counter = 0;        // total voltage overvoltage fault counter

// LTC6811_2 OBJECT DECLARATIONS
LTC6811_2 ic[8];

// CAN OBJECT AND VARIABLE DECLARATIONS
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN;
CAN_message_t msg;

// BMS CAN MESSAGE AND STATE MACHINE OBJECT DECLARATIONS
BMS_status bms_status; //Message class that contains flags for AMS errors as well as a variable encoding the current state of the AMS (charging vs. discharging)


void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(6, LOW); //write Teensy_OK pin high
  Serial.begin(115200);
  SPI.begin();
  CAN.setBaudRate(500000);
  CAN.begin();
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
}
int i = 0;
void loop() {

  // put your main code here, to run repeatedly:

  if (i < 20) {
    if (bms_status.get_state() == BMS_STATE_DISCHARGING) {
      Serial.println("BMS state: Discharging\n");
    }
    Serial.println(i, DEC);
    read_voltages();
    print_cells();
    i++;
  }
}

// read voltages from all eight LTC6811-2; voltages are read in with units of 100μV
void read_voltages() {
  total_voltage = 0;
  uint16_t min_voltage = 65535;
  int min_voltage_location[2]; // [0]: IC#; [1]: Cell#
  uint16_t max_voltage = 0;
  int max_voltage_location[2]; // [0]: IC#; [1]: Cell#

  Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x1F, false, false, vuv, vov, (uint16_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group
  for (int i = 0; i < 8; i++) {
    Serial.print("Loop #: "); Serial.println(i, DEC);
    ic[i].wakeup();
    Serial.println("starting wrcfga");
    ic[i].wrcfga(configuration);
    Serial.println("wrfcga configuration");
    uint8_t *wrfcga_buf = configuration.buf();
    Serial.println(wrfcga_buf[0], BIN);
    Serial.println(wrfcga_buf[1], BIN);
    Serial.println(wrfcga_buf[2], BIN);
    Serial.println(wrfcga_buf[3], BIN);
    Serial.println(wrfcga_buf[4], BIN);
    Serial.println(wrfcga_buf[5], BIN);
    Reg_Group_Config reg_group_config = ic[i].rdcfga();
    Serial.println("wrfcga readback: ");
    Serial.println(reg_group_config.buf()[0], BIN);
    Serial.println(reg_group_config.buf()[1], BIN);
    Serial.println(reg_group_config.buf()[2], BIN);
    Serial.println(reg_group_config.buf()[3], BIN);
    Serial.println(reg_group_config.buf()[4], BIN);
    Serial.println(reg_group_config.buf()[5], BIN);
    Serial.println("starting adcv");
    ic[i].adcv(static_cast<CELL_SELECT>(0));
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
        buf = reg_group_d.buf();
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
  // detect any uv fault conditions, set appropriate error flags, and print relevant message to console
  if (min_voltage < MIN_VOLTAGE) {
    uv_fault_counter++;
    Serial.print("UNDERVOLTAGE FAULT: "); Serial.print("IC #: "); Serial.print(min_voltage_location[0]); Serial.print("\tCell #: "); Serial.print(min_voltage_location[1]); Serial.print("\tConsecutive fault #: "); Serial.println(uv_fault_counter);
    if (uv_fault_counter > MAX_SUCCESSIVE_FAULTS) {
      bms_status.set_undervoltage(true);
    }
  } else {
    uv_fault_counter = 0;
    bms_status.set_undervoltage(false);
  }
  // detect any ov fault conditions, set appropriate error flags, and print relevant message to console
  if (max_voltage > MAX_VOLTAGE) {
    ov_fault_counter++;
    Serial.print("OVERVOLTAGE FAULT: "); Serial.print("IC #: "); Serial.print(max_voltage_location[0]); Serial.print("\tCell #: "); Serial.print(max_voltage_location[1]); Serial.print("\tConsecutive fault #: "); Serial.println(ov_fault_counter);
    if (uv_fault_counter > MAX_SUCCESSIVE_FAULTS) {
      bms_status.set_overvoltage(true);
    }
  } else {
    ov_fault_counter = 0;
    bms_status.set_overvoltage(false);

  }
  // detect any pack ov fault conditions, set appropriate error flags, and print relevant message to console
  if (total_voltage > MAX_TOTAL_VOLTAGE) {
    pack_ov_fault_counter++;
    Serial.print("PACK OVERVOLTAGE FAULT: "); Serial.print("\tConsecutive fault #: "); Serial.println(pack_ov_fault_counter);
    if (pack_ov_fault_counter > MAX_SUCCESSIVE_FAULTS) {
      bms_status.set_total_voltage_high(true);
    }
  } else {
    pack_ov_fault_counter = 0;
    bms_status.set_total_voltage_high(false);
  }
}

// Read GPIO registers from LTC6811-2; Process temperature and humidity data from relevant GPIO registers
void read_gpio() {
  Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x1F, false, false, vuv, vov, (uint16_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group
  for (int i = 0; i < 8; i++) {
    ic[i].wakeup();
    Serial.println("starting wrcfga");
    ic[i].wrcfga(configuration);
    Serial.println("starting adax");
    ic[i].adax(static_cast<GPIO_SELECT>(0));
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
      }
    }
  }
}

// parse incoming CAN messages for CCU status message
void parse_can_message() {
  while (CAN.read(msg)) {
    if (msg.id == ID_CCU_STATUS) {
      if (bms_status.get_state() == BMS_STATE_DISCHARGING) {
        bms_status.set_state(BMS_STATE_CHARGING);
      }
    }
  }
}

void print_cells() {
  Serial.print("Total pack voltage: "); Serial.print(total_voltage / 10000.0, 4); Serial.println("V");
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
