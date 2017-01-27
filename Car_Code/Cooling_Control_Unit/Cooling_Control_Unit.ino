#include <FlexCan.h> // import teensy library
#include <HyTech17.h>

#define DRIVE_STATE 8 //PLACEHOLDER, need to decide right value for drive state from TCU
//TODO: ports

FlexCAN CAN(500000);
static CAN_message_t msg;

// State - use HyTech library
uint8_t state;
bool started =  false;

// setup code
void setup() {
    Serial.begin(115200); // init serial

    CAN.begin();
    Serial.println("CAN system and serial communication initialized");
}

// loop code
void loop() {
    while (CAN.read(msg)) {
        if (!started && msg.id == 0xD1 && msg.buf[1] == DRIVE_STATE) {
            started = true;
            //initial start up of car, so start up all fans and pump to some initial speed
        }
        if (started) {
            // TODO Read AVG_TEMP message from components and utilize PWM to change
            // fan speed based on temps
            // Code not yet written on BMS to send temperatures
    
            // TODO Handle CAN messages from other components
            // Placeholder code
            // Will update when I figure out ports for fans, how to increase
            // fan speed, which fans to increase the speed of, and how to adjust
            // fan speed based on temperature
            switch (msg.id) {
            case 0x51: // Main ECU thermTemp
                if (msg.buf[0] > 40) {
                    // Increase fan speed with PWM
                } else {
                    // Decrease fan speed with PWM
                }
                break;
            case ID_TCU_STATUS: // TCU thermTemp
                if (msg.buf[0] > 40) {
                    // Increase fan speed with PWM
                } else {
                    // Decrease fan speed with PWM
                }
                break;
            default
                sendCanUpdate();
                break;
            }
        }
    }
}

int sendCanUpdate() {
    //TODO write CAN update code
}
