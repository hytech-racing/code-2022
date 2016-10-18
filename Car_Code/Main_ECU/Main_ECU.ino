#include <FlexCAN.h> // import teensy library

FlexCAN CAN(500000);
static CAN_message_t msg;
int OKHS = 0; // voltage after calculation
int DISCHARGE_OK = 0; // voltage after calculation
int GLVbattery = 0; // voltage after calculation
int shutdownCircuit = 0; // voltage after calculation
bool OKHSfault = false; // fault status of OKHS, fault if true
bool DISCHARGE_OKfault = false; // fault status of DISCHARGE_OK, fault if true
bool startPressed = false; // true if start button is pressed
int thermTemp = 0; // temperature of onboard thermistor (after calculation)
int timer = 0; // needed to check timer
bool startupDone = false; // true when reached drive state
bool softwareFault = false; // true when software fault found

// timer
unsigned long timer; // use timer = millis() to get time, and compare in ms

const int OKHS_PIN = 0;
const int BMS_OK_PIN = 1;

enum State { GLVinit=0, waitIMDBMS, waitDriver, AIRClose, fatalFault, drive }; // NOTE: change and update
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
        switch (curState) {
            case GLVinit:
                curState = waitIMDBMS; //going straight to waitIMD unti further notice
            case waitIMDBMS:
                if (softwareFault) {
                    curState = fatalFault;
                }
                if (DISCHARGE_OK >= 50) { // if BMS is high NOTE: change value
                    if (OKHS >= 50) { // if IMD is also high NOTE: change value
                        curState = waitStartButton; // both BMD and IMD are high, wait for start button press
                    }
                }
                break;
            case waitDriver:
                /*can message for start button press received*/
                curState = closeLatch;
                break;
            case AIRClose:
            case fatalFault:
            case drive:

        }
    } else {

    }
}
