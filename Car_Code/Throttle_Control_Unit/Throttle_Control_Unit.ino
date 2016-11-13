#include <FlexCAN.h> // import teensy library
#include <Metro.h>

//ports
#define BRAKE_ANALOG_PORT 3 //analog port of brake sensor
#define THROTTLE_PORT_1 6 //first throttle sensor port
#define THROTTLE_PORT_2 9 //second throttle sensor port
// TODO: These values need to be determined from testing
//constants
#define MIN_THROTTLE_1 0//compare pedal travel
#define MAX_THROTTLE_1 1024
#define MIN_THROTTLE_2 0
#define MAX_THROTTLE_2 1024
#define MIN_BRAKE 0
#define MAX_BRAKE 1024

//Description of Throttle Control Unit
//Senses the angle of each pedal to determine safely how much torque the motor controller should produce with the motor
//Reads a signal from the Brake System Plausibility Device located on the same board to sense if the BSPD has detected a fault.

FlexCAN CAN(500000);
static CAN_message_t msg;

int voltageThrottlePedal1 = 0; //voltage of 1st throttle
int voltageThrottlePedal2 = 0; //voltage of 2nd throttle
int voltageBrakePedal = 0;//voltage of brakepedal

// additional values to report
bool implausibilityStatus = false; // for pedal not brake
bool throttleCurve = false; // false -> normal, true -> boost
float thermTemp = 0.0; // temperature of onboard thermistor
bool brakePlausibility = false; // fault if BSPD signal too low - still to be designed
bool brakePedalActive = false; // true if brake is considered pressed

//FSAE requires that torque be shut off if an implausibility persists for over 100 msec (EV2.3.5).
//A deviation of more than 10% pedal travel between the two throttle sensors
//A failure of position sensor wiring which can cause an open circuit, short to ground, or short to sensor power.
bool torqueShutdown = false; //

// FUNCTION PROTOTYPES
void readValues();
bool checkDeactivateTractiveSystem();
int canUpdate();

//TCU states
enum TCU_STATE{
    INITIAL_POWER,
    SHUTDOWN_CIRC_INIT,
    TS_ACTIVE,
    TS_NOT_ACTIVE,
    INVERTER_ENABLE,
    RTD_WAIT,
    RTD
} state;

// timer
Metro stateTimer = Metro(500); // Used for how often to send state
Metro updateTimer = Metro(500); // Read in values from pins
Metro implausibilityTimer = Metro(50); // Used for throttle error check
Metro throttleTimer = Metro(500); // Used for sending commands to Motor Controller

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
    if (updateTimer.check()) {
        readValues();
        updateTimer.reset();
    }
    if (implausibilityTimer.check()) {
        checkDeactivateTractiveSystem();
        implausibilityTimer.reset();
    }
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
            break;
    }
}
    //Error Message Instructions
    //an error message should be sent out on CAN Bus detailing which implausibility has been detected.
    //periodically sent until the implausibility ceases to exist.
    //If the implausibility ceases, a corresponding message should be sent on CAN Bus.
    //If an implausibility ceases to be detected, normal throttle controls should be reinstated
    //i.e. the vehicle does not need to be restarted to reset an implausibility fault.
/*
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

bool checkDeactivateTractiveSystem() {
    // Check for errors - AKA implausibility checking
    // Throttle 10% check
    float deviationCheck = ((float) voltageThrottlePedal1) / ((float) voltageThrottlePedal2);
    if (deviationCheck > 1.10 || (1 / deviationCheck) > 1.10) {
        // TODO: implausibility
    }
    // Checks for failure of position sensor wiring
    // Check for open circuit or short to ground
    if (voltageThrottlePedal1 < MIN_THROTTLE_1 || voltageThrottlePedal2 < MIN_THROTTLE_2) {
        //TODO: implausibility
    }
    // Check for short to power
    if (voltageThrottlePedal1 > MAX_THROTTLE_1 || voltageThrottlePedal2 > MAX_THROTTLE_2) {
        //TODO: implausibility
    }
    // Check brake pedal sensor
    if (voltageBrakePedal > MAX_BRAKE || voltageBrakePedal < MIN_BRAKE) {
        //TODO: implausibility
    }
    return true;
}

<<<<<<< Updated upstream
void checkBrakeImplausibility() {
    // TODO: TCU should read in signal from BSPD
    // Fault occurs when signal is too low
    // NOT when fault on brake pedal sensor
}
=======
int sendCanUpdate(){

}


>>>>>>> Stashed changes
