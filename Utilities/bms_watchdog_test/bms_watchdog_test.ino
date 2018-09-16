/*
 * Yvonne Yeh
 * Activate the watchdog test mode on the BMS
 * 
 * When the BMS receives a CAN message with ID `ID_FH_WATCHDOG_TEST`,
 * the BMS will stop interacting with the watchdog,
 * causing it to unlatch the BMS_OK signal.
 */

#include <HyTech_FlexCAN.h>
#include "HyTech_CAN.h"

static CAN_message_t msg;

void setup() {
  Serial.begin(115200); // Init serial for PC communication
  Can0.begin(500000); // Init CAN for vehicle communication
  Serial.println("Initializing remote BMS watchdog test");
}

void loop() {
   msg.id = ID_FH_WATCHDOG_TEST;
   msg.len = 1;
   Can0.write(msg);
}
