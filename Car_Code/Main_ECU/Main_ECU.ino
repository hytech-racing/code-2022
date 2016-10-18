#include <FlexCAN.h> // import teensy library

FlexCAN CAN(500000);
static CAN_message_t msg;
int OKHS = 0; // voltage after calculation
int DISCHARGE_OK = 0; // voltage after calculation
int GLVbattery = 0; // voltage after calculation
int shutdownCircuit = 0; // voltage after calculation
boolean OKHSfault = false; // fault status of OKHS, fault if true
boolean DISCHARGE_OKfault = false; // fault status of DISCHARGE_OK, fault if true
boolean startPressed = false; // true if start button is pressed
int thermTemp = 0; // temperature of onboard thermistor (after calculation)
int timer = 0; // needed to check timer
boolean startupDone = false; // true when reached drive state

// timer
unsigned long timer; // use timer = millis() to get time, and compare in ms

const int OKHS_PIN = 0;
const int BMS_OK_PIN = 1;

enum State { GLVinit=0, waitIMD, waitBMS, waitStartButton, closeLatch, openLatch, AIROpen, AIRClose, waitInverter, readySoundOn, drive };
State curState = GLVinit; // curState is current state
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
                curState = waitIMD; //going straight to waitIMD unti further notice
            case waitIMD:
            case waitBMS:
                if (DISCHARGE_OK >= 50) { // if BMS is high
                    if (OKHS >= 50) { // if IMD is also high
                        curState = waitStartButton; // both BMD and IMD are high, wait for start button press
                    } else { // if IMD is low
                        curState = waitIMD; // BMS is high but IMDis low, wait for IMD high
                    }
                }
                break;
            case waitStartButton:
                /*can message for start button press received*/
                curState = closeLatch;
            case closeLatch:
            case openLatch:
                // Open latch (?)
                // go to AIROpen if shutdown ciruit or precharge fault
                // go to AIRClosed if shutdown circuit closed and no precharge fault
            case AIROpen:
                /*CAN message for faults fixed*/
                curState = AIRClose;
            case AIRClose:
            case waitInverter:
                // Wait for motor controller inverter
                // go to readySoundOn if inverter enabled
                // go to AIROpen if Cockpit BRB open
            case readySoundOn:
            case drive:

        }
    } else {

    }
}
