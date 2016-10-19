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
bool startupDone = false; // true when reached drive state
bool softwareFault = false; // true when software fault found

// Values to check if IMD, BMS high
const int IMD_High = 50;
const int BMS_High = 50;

// timer
unsigned long timer; // use timer = millis() to get time, and compare in ms

const int OKHS_PIN = 0;
const int BMS_OK_PIN = 1;

enum State { GLVinit=0, waitIMDBMS, waitDriver, AIRClose, fatalFault, drive }; // NOTE: change and update
State curState = GLVinit; // curState is current state

//FUNCTION PROTOTYPES
bool checkIMDBMS();
bool checkFatalFault();

// setup code
void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
}

// loop code
void loop() {
    //check CAN for a message for software shutdown
    if (!startupDone) {
        switch (curState) {
            case GLVinit:
                curState = waitIMDBMS; //going straight to waitIMD unti further notice
                break;
            case waitIMDBMS:
                if (softwareFault) {
                    curState = fatalFault;
                } else {
                    if (DISCHARGE_OK >= BMS_High) { // if BMS is high
                        if (OKHS >= IMD_High) { // if IMD is also high 
                            curState = waitStartButton; // both BMD and IMD are high, wait for start button press
                        }
                    }
                }
                break;
            case waitDriver:
                if (checkFatalFault()) {
                    curState = fatalFault;
                } else {
                    /*can message for start button press received*/
                    curState = closeLatch;
                }
                break;
            case AIRClose: // equivalent to VCCAIR in Google Doc state diagram
                if (checkFatalFault()) {
                    curState = fatalFault;
                }
                break;
            case fatalFault:
            case drive:

        }
    } else {

    }
}

boolean checkIMDBMS() {
   DISCHARGE_OK = analogRead(BMS_OK_PIN);
   OKHS = analogRead(OKHS_PIN);
   DISCHARGE_OK = DISCHARGE_OK / 67.7;
   OKHS = OKHS / 67.7;
}

bool checkFatalFault() { // returns true if fatal fault found ()
    if (OKHS >= IMD_High && DISCHARGE_OK >= BMS_High && !softwareFault) {
        return false;
    } else {
        return true;
    }
}
