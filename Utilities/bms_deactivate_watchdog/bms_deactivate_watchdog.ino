#include <FlexCAN.h>
#include "HyTech_CAN.h"
#include "UserInterface.h"

FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Init serial for PC communication
  CAN.begin(); // Init CAN for vehicle communication
  Serial.println("Enter any key to deactivate BMS watchdog");
}

void loop() {
  // put your main code here, to run repeatedly:
  /*if (Serial.available()) {         // Check for user input
    uint32_t user_command;
    user_command = read_int();      // Read the user command
    Serial.println(user_command); 
    
    msg.id = ID_FH_WATCHDOG_TEST;
    msg.len = 1;
    CAN.write(msg);
    Serial.println("BMS watchdog deactivated");
  }*/
   msg.id = ID_FH_WATCHDOG_TEST;
   msg.len = 1;
   CAN.write(msg);
}
