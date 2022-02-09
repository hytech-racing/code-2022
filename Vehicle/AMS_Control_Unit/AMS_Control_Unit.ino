/* ACU CONTROL UNIT CODE
 * The AMS Control Unit code is used to control and communicate with Analog Devices LTC6811-2 battery stack monitors, per the HyTech Racing HT06 Accumulator Design. 
 * It also handles CAN communications with the mainECU and energy meter, performs coulomb counting operations, and drives a watchdog timer on the ACU. 
 * See LTC6811_2.cpp and LTC6811-2 Datasheet provided by Analog Devices for more details. 
 * Author: Zekun Li 
 * Version: 0.1
 * Since: 02/07/2022
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

// VARIABLE DECLARATIONS
uint16_t vuv = 1874; // 3V           // Minimum voltage value following datasheet formula: Comparison Voltage = (VUV + 1) • 16 • 100μV
uint16_t vov = 2625; // 4.2V         // Maximum voltage value following datasheet formula: Comparison Voltage = VOV • 16 • 100μV 
uint16_t cell_voltages[TOTAL_IC][12]; // 2D Array to hold cell voltages being read in;

// LTC6811_2 OBJECT DECLARATIONS
LTC6811_2 ic[8]; 
Reg_Group_Config configuration_register = Reg_Group_Config((uint8_t) 0x0, false, false, vuv, vov, (uint8_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group

// CAN OBJECT AND VARIABLE DECLARATIONS
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN;
CAN_message_t msg;

// BMS CAN MESSAGE AND STATE MACHINE OBJECT DECLARATIONS
BMS_status bms_status; //Message class that contains flags for AMS errors as well as a variable encoding the current state of the AMS (charging vs. discharging)


void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(6, HIGH); //write Teensy_OK pin high
  Serial.begin(115200);
  SPI.begin();
  CAN.setBaudRate(500000);
  CAN.begin();
  for (int i = 0; i < 64; i++) { // Fill all filter slots with Charger Control Unit message filter (CAN controller requires filling all slots)
    CAN.setMBFilter(static_cast<FLEXCAN_MAILBOX>(i), ID_CCU_STATUS); // Set CAN mailbox filtering to only watch for charger controller status CAN messages
  }

   // add 8 instances of LTC6811_2 to the object array, each addressed appropriately
   for (int i = 0; i < 8; i++) {
    ic[i] = LTC6811_2(i);  
    ic[i].init_PEC15_Table();
   }

   /* Initialize ACU state machine
    *  The state machine has two states, dependent on whether or not the charger controller is talking to the ACU. 
    *  The state machine initializes to the DISCHARGING state
    */
   bms_status.set_state(BMS_STATE_DISCHARGING);
}
int i = 0;
void loop() {
  
  // put your main code here, to run repeatedly:
  
  if (i < 1) {
    if (bms_status.get_state() == BMS_STATE_DISCHARGING) {
      Serial.println("BMS state: Discharging\n");
    }
    Serial.println(i, DEC);
    read_voltages();
    print_cells();
  }
  i++;
}

void read_voltages() {
  Reg_Group_Config configuration = Reg_Group_Config((uint8_t) 0x0, false, false, vuv, vov, (uint16_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group
  for (int i = 0; i < 8; i++) {
    ic[i].wakeup();
    Serial.println("starting wrcfga");
    ic[i].wrcfga(configuration);
    Serial.println("starting adcv");
    ic[i].adcv(static_cast<CELL_SELECT>(0));
    delay(202); // delay 202 milliseconds to wait for ADC conversion to finish for 26Hz mode
    Reg_Group_Cell_A reg_group_a = ic[i].rdcva();
    Reg_Group_Cell_B reg_group_b = ic[i].rdcvb();
    Reg_Group_Cell_C reg_group_c = ic[i].rdcvc();
    Reg_Group_Cell_D reg_group_d = ic[i].rdcvd();
    for (int j = 0; j < 12; j+= 3) {
      uint8_t *buf;
      if (j == 0) {
        buf = reg_group_a.buf();
      } else if (j == 3) {
        buf = reg_group_b.buf();
      } else if (j ==6) {
        buf = reg_group_c.buf();
      } else if (j == 9) {
        buf = reg_group_d.buf();
      }
      for (int k = 0; k < 3; k++) {
        cell_voltages[i][j + k] = buf[2*k+1] << 8 | buf[2*k];
      }
    }
  }
}

void print_cells() {
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\t\t\t\tRaw Cell Voltages\t\t\t\t\t\t\tCell Status (Ignoring or Balancing)");
    Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8\t\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int cell = 0; cell < EVEN_IC_CELLS; cell++) {
            Serial.print(cell_voltages[ic][cell] / 10000.0, 4); Serial.print("V\t");
        }
        Serial.print("\t");
        Serial.println();
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
