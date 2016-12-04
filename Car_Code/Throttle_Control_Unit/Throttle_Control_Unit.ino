#include <FlexCAN.h> // import teensy library
#include <Metro.h>
#include <HyTech17.h>

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
bool throttleImplausibility = false; // for throttle, not brake
bool throttleCurve = false; // false -> normal, true -> boost
float thermTemp = 0.0; // temperature of onboard thermistor
bool brakeImplausibility = false; // fault if BSPD signal too low - still to be designed
bool brakePedalActive = false; // true if brake is considered pressed

// FSAE requires that torque be shut off if an implausibility persists for over 100 msec (EV2.3.5).
// TODO check for checkDeactivateTractiveSystem() (when true) for 100ms of true
//A deviation of more than 10% pedal travel between the two throttle sensors
//A failure of position sensor wiring which can cause an open circuit, short to ground, or short to sensor power.
bool torqueShutdown = false;

// FUNCTION PROTOTYPES
void readValues();
bool checkDeactivateTractiveSystem();
int sendCanUpdate();

// State - use HyTech library
uint8_t state;

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
    state = TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED;
}


//FSAE requires that torque be shut off if an implausibility persists for over 100 msec (EV2.3.5).
    //A deviation of more than 10% pedal travel between the two throttle sensors
    //A failure of position sensor wiring which can cause an open circuit, short to ground, or short to sensor power.

    //To detect a position sensor wiring failure
    //find the ranges of values coming from each sensor during normal operation of the foot pedals
    //Any values outside of these ranges could be caused by an open circuit, short to ground, or short to sensor power.

    void loop() {
        while (CAN.read(msg)) {
            PCU_status pcu_status(msg.buf);
            if (msg.id == ID_PCU_STATUS) {
                if (pcu_status.get_bms_fault()) {
                    Serial.println("BMS Fault detected");
                }
                if (pcu_status.get_imd_fault()) {
                    Serial.println("IMD Fault detected");
                }
            }
            switch (pcu_status.get_state()) {
                case PCU_STATE_WAITING_BMS_IMD:
                    set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
                    break;
                case PCU_STATE_WAITING_DRIVER:
                    set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
                    break;
                case PCU_STATE_LATCHING:
                    set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
                    break;
            }
        }

        if (updateTimer.check()) {
            readValues();
            updateTimer.reset();
        }
        if (implausibilityTimer.check()) {
            checkDeactivateTractiveSystem();
            implausibilityTimer.reset();
        }
        if(stateTimer.check()){
            sendCanUpdate();
            stateTimer.reset();
        }
        switch(state) {
            //TODO: check if reqs are met to move to each state
            case TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED:
                state = TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE;
                break;
            case TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
                state = TCU_STATE_TRACTIVE_SYSTEM_ACTIVE;
                break;
            case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
                state = TCU_STATE_ENABLING_INVERTER;
                break;
            case TCU_STATE_ENABLING_INVERTER:
                state = TCU_STATE_WAITING_READY_TO_DRIVE_SOUND;
                break;
            case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
                state = TCU_STATE_READY_TO_DRIVE;
                break;
            case TCU_STATE_READY_TO_DRIVE:
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
        throttleImplausibility = true;
    } else if (voltageThrottlePedal1 < MIN_THROTTLE_1 || voltageThrottlePedal2 < MIN_THROTTLE_2) {
        // Checks for failure of position sensor wiring
        // Check for open circuit or short to ground
        throttleImplausibility = true;

    } else if (voltageThrottlePedal1 > MAX_THROTTLE_1 || voltageThrottlePedal2 > MAX_THROTTLE_2) {
        // Check for short to power
        throttleImplausibility = true;
    } else {
        // No throttle implausibility detected
        throttleImplausibility = false;
    }

    // Check brake pedal sensor
    if (voltageBrakePedal > MAX_BRAKE || voltageBrakePedal < MIN_BRAKE) {
        brakeImplausibility = true;
    } else {
        // No brake implausibility detected
        brakeImplausibility = false;
    }

    // return true if any implausibility present
    if (throttleImplausibility || brakeImplausibility) {
        return true;
    } else {
        return false;
    }
}

int sendCanUpdate(){
    short shortThrottle1 = (short) voltageThrottlePedal1 * 100;
    short shortThrottle2 = (short) voltageThrottlePedal2 * 100;
    short shortBrake = (short) voltageBrakePedal * 100;
    short shortTemp = (short) thermTemp * 100;

    msg.id = ID_TCU_STATUS;
    msg.len = 8;

    memcpy(&msg.buf[0], &shortThrottle1, sizeof(short));
    memcpy(&msg.buf[2], &shortThrottle2, sizeof(short));
    memcpy(&msg.buf[4], &shortBrake, sizeof(short));
    memcpy(&msg.buf[6], &shortTemp, sizeof(short));

    int temp1 = CAN.write(msg);

    byte statuses = state;

    if(throttleImplausibility) statuses += 16;
    if(throttleCurve) statuses += 32;
    if(brakeImplausibility) statuses += 64;
    if(brakePedalActive) statuses += 128;

    msg.id = ID_TCU_STATUS;
    msg.len = 1;
    msg.buf[0] = statuses;

    int temp2 = CAN.write(msg);

    return temp1 + temp2; // used for error checking?
}

void set_state(uint8_t new_state) {
    if (state == new_state) {
        return;
    }
    uint8_t old_state = state;
    state = new_state;
    // TODO handle state transitions
}
