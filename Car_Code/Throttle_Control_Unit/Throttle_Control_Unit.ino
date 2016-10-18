#include <FlexCAN.h> // import teensy library

//Description of Throttle Control Unit
//Senses the angle of each pedal to determine safely how much torque the motor controller should produce with the motor
//Reads a signal from the Brake System Plausibility Device located on the same board to sense if the BSPD has detected a fault.

FlexCAN CAN(500000);
static CAN_message_t msg;
int voltageBrakePedal = 0;//voltage of brakepedal
int voltageThrottlePedal1 = 0; //voltage of 1st throttle
int voltageThrottlePedal2 = 0; //voltage of 2nd throttle
const int BRAKE_ANALOG_PORT = 3; //analog port of brake sensor
const int THROTTLE_PORT_1 = 6; //first throttle sensor port
const int THROTTLE_PORT_2 = 9; //second throttle sensor port

//FSAE requires that torque be shut off if an implausibility persists for over 100 msec (EV2.3.5).
//A deviation of more than 10% pedal travel between the two throttle sensors
//A failure of position sensor wiring which can cause an open circuit, short to ground, or short to sensor power.












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

//FUNCTION PROTOTYPES
boolean checkIMDBMS();

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

boolean checkIMDBMS() {
   DISCHARGE_OK = analogRead(BMS_OK_PIN);
   OKHS = analogRead(OKHS_PIN);
   DISCHARGE_OK = DISCHARGE_OK / 67.7;
   OKHS = OKHS / 67.7;

}

