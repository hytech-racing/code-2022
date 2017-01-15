#include <FlexCan.h> // import teensy library
#include <HyTech17.h>

FlexCAN CAN(500000);
static CAN_message_t msg;

// State - use HyTech library
uint8_t state;

// setup code
void setup() {
    Serial.begin(115200); // init serial

    CAN.begin();
    Serial.println("CAN system and serial communication initialized");
}

// loop code
void loop() {
    while (CAN.read(msg)) {
        //TODO Read AVG_TEMP message from BMS and utilize PWM to change fan
        //speed based on temps
        //Code not yet written on BMS to send temperatures
    }
}

int sendCanUpdate() {
    //TODO write CAN update code
}
