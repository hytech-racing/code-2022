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

// LTC6811_2 OBJECT DECLARATIONS
LTC6811_2 ic_0 = LTC6811_2(0);
LTC6811_2 ic_1 = LTC6811_2(1);
LTC6811_2 ic_2 = LTC6811_2(2);
LTC6811_2 ic_3 = LTC6811_2(3);
LTC6811_2 ic_4 = LTC6811_2(4);
LTC6811_2 ic_5 = LTC6811_2(5);
LTC6811_2 ic_6 = LTC6811_2(6);
LTC6811_2 ic_7 = LTC6811_2(7);
Reg_Group_Config configuration_register = Reg_Group_Config((uint8_t) 0x0, false, false, vuv, vov, (uint8_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group

// CAN OBJECT AND VARIABLE DECLARATIONS
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN;
CAN_message_t msg;

// BMS CAN MESSAGE AND STATE MACHINE OBJECT DECLARATIONS
BMS_status bms_status; //Message class that contains flags for AMS errors as well as a variable encoding the current state of the AMS (charging vs. discharging)


void setup() {
  // put your setup code here, to run once:
  digitalWrite(6, HIGH) //write Teensy_OK pin high
  Serial.begin(115200);
  CAN.setBaudRate(500000);
  CAN.begin();
  for (int i = 0; i < 64; i++) { // Fill all filter slots with Charger Control Unit message filter (CAN controller requires filling all slots)
        CAN.setMBFilter(static_cast<FLEXCAN_MAILBOX>(i), ID_CCU_STATUS); // Set CAN mailbox filtering to only watch for charger controller status CAN messages
   }

   /* Initialize ACU state machine
    *  The state machine has two states, dependent on whether or not the charger controller is talking to the ACU. 
    *  The state machine initializes to the DISCHARGING state
    */
   bms_status.set_state(BMS_STATE_DISCHARGING);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void read_voltages() {
  Reg_Group_Config configuration_register = Reg_Group_Config((uint8_t) 0x0, false, false, vuv, vov, (uint8_t) 0x0, (uint8_t) 0x1); // base configuration for the configuration register group
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
