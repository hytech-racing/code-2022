#include <FlexCAN.h> // import teensy library

FlexCAN CAN(500000);
static CAN_message_t msg;

// setup code
void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
}

// loop code
void loop() {
  
}
