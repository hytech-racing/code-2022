#include <FlexCAN.h> // import teensy library

FlexCAN CAN(500000);
static CAN_message_t msg;
int OKHS = 0;
int DISCHARGE_OK = 0;
boolean startPressed = false;
int timer = 0;
boolean startupDone = false;

enum state { GLVinit=0, waitIMD, waitBMS, waitStartButton, closeLatch, openLatch, AIROpen, AIRClose, waitInverter, readySoundOn, drive };
state = GLVinit;
// setup code
void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
}

// loop code
void loop() {
    if (!startupDone) {
        switch (state) {
            case GLVinit: 
            case waitIMD:
            case waitBMS:
            case waitStartButton:
            case closeLatch:
            case openLatch:
            case AIROpen:
            case AIRClose:
            case waitInverter:
            case readySoundOn:
            case drive:
            
        }
    } else {
        
    }
}
