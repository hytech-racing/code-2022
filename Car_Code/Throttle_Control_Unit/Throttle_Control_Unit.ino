#include <FlexCAN.h> // import teensy library

//Description of Throttle Control Unit
//Senses the angle of each pedal to determine safely how much torque the motor controller should produce with the motor
//Reads a signal from the Brake System Plausibility Device located on the same board to sense if the BSPD has detected a fault.

FlexCAN CAN(500000);
static CAN_message_t msg;
unsigned long timer; // use timer = millis() to get time, and compare in ms
int voltageThrottlePedal1 = 0; //voltage of 1st throttle
int voltageThrottlePedal2 = 0; //voltage of 2nd throttle
int voltageBrakePedal = 0;//voltage of brakepedal
const int BRAKE_ANALOG_PORT = 3; //analog port of brake sensor
const int THROTTLE_PORT_1 = 6; //first throttle sensor port
const int THROTTLE_PORT_2 = 9; //second throttle sensor port
const int MIN_THROTTLE_1 = 0;//compare pedal travel
const int MAX_THROTTLE_1 = 1024;
const int MIN_THROTTLE_2 = 0;
const int MAX_THROTTLE_2 = 1024;
const int MIN_BRAKE = 0;
const int MAX_BRAKE = 1024;

// additional values to report
bool implausibilityStatus = false;
bool throttleCurve = false; // false -> normal, true -> boost
float thermTemp = 0.0; // temperature of onboard thermistor
bool brakePlausibility = false; // falt if BSPD signal too low
bool brakePedalActive = false; // true if brake is considered pressed

//FSAE requires that torque be shut off if an implausibility persists for over 100 msec (EV2.3.5).
//A deviation of more than 10% pedal travel between the two throttle sensors
//A failure of position sensor wiring which can cause an open circuit, short to ground, or short to sensor power.
bool torqueShutdown = false; //

// Throttle Control Unit states
enum State { GLVinit=0, waitSDCircInit, tracSysActive, enablingInv, waitRtD, readyToDrive, tracSysNotActive};
State curState = GLVinit;

// FUNCTION PROTOTYPES
void readValues();
bool checkDeactivateTractiveSystem();

//STATE TRACKER
enum TCU_STATE{
    INITIAL_POWER,
    SHUTDOWN_CIRC_INIT,
    TS_ACTIVE,
    TS_NOT_ACTIVE,
    INVERTER_ENABLE,
    RTD_WAIT,
    RTD     
} state;

// setup code
void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
    //To detect an open circuit
    //enable the pullup resistor on the Teensy input pin >>>
    pinMode(BRAKE_ANALOG_PORT, INPUT_PULLUP);
    pinMode(THROTTLE_PORT_1, INPUT_PULLUP);
    pinMode(THROTTLE_PORT_2, INPUT_PULLUP);
    //open circuit will show a high signal outside of the working range of the sensor.
    state = INITIAL_POWER;
}


//FSAE requires that torque be shut off if an implausibility persists for over 100 msec (EV2.3.5).
    //A deviation of more than 10% pedal travel between the two throttle sensors
    //A failure of position sensor wiring which can cause an open circuit, short to ground, or short to sensor power.

    //To detect a position sensor wiring failure
    //find the ranges of values coming from each sensor during normal operation of the foot pedals
    //Any values outside of these ranges could be caused by an open circuit, short to ground, or short to sensor power.

void loop() {
    switch(state) {
        //TODO: check if reqs are met to move to each state
        case INITIAL_POWER:
            state = SHUTDOWN_CIRC_INIT;
            break;
        case SHUTDOWN_CIRC_INIT:
            state = TS_ACTIVE;
            break;
        case TS_ACTIVE:
            state = INVERTER_ENABLE;
            break;
        case INVERTER_ENABLE:
            state = RTD_WAIT;
            break;
        case RTD_WAIT:
            state = RTD;
            break;
        case RTD:
            readValues();
            checkDeactivateTractiveSystem();
            break;
    }
}
    //Error Message Instructions
    //an error message should be sent out on CAN Bus detailing which implausibility has been detected.
    //periodically sent until the implausibility ceases to exist.
    //If the implausibility ceases, a corresponding message should be sent on CAN Bus.
    //If an implausibility ceases to be detected, normal throttle controls should be reinstated
    //i.e. the vehicle does not need to be restarted to reset an implausibility fault.
/* LOL FUCK THIS
int giveError(int errorID) {
   CAN.write(errorID)
   return 1; //placeholder
}
*/
void readValues() {
    voltageThrottlePedal1 = analogRead(THROTTLE_PORT_1);
    voltageThrottlePedal2 = analogRead(THROTTLE_PORT_2);
    voltageBrakePedal = analogRead(BRAKE_ANALOG_PORT);
    //TODO: decide/set torque values for input values
}

bool checkDeactivateTractiveSystem() { //
    //Check for errors
    if(voltageThrottlePedal1 / voltageThrottlePedal2 > 1.1 || voltageThrottlePedal1 / voltageThrottlePedal2 < 0.9) {
        //TODO: SHUTDOWN TORQUE - PEDALS NOT AGREEING
    }
    if (voltageThrottlePedal1 > MAX_THROTTLE_1 || voltageThrottlePedal1 < MIN_THROTTLE_1) {
        //TODO: SHUTDOWN TORQUE - PEDAL 1 CRAZY
    }
    if (voltageThrottlePedal2 > MAX_THROTTLE_2 || voltageThrottlePedal2 < MIN_THROTTLE_2) {
        //TODO: SHUTDOWN TORQUE - PEDAL 2 CRAZY
    }
    if (voltageBrakePedal > MAX_BRAKE || voltageBrakePedal < MIN_BRAKE) {
        //TODO: SHUTDOWN TORQUE - BRAKE CRAZY
    }
    return true;
}
