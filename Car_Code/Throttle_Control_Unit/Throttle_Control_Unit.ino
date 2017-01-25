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
int sendCANUpdate();

// State - use HyTech library
uint8_t state;

// timer
Metro CANUpdateTimer = Metro(500); // Used for how often to send state
Metro updateTimer = Metro(500); // Read in values from pins
Metro implausibilityTimer = Metro(50); // Used for throttle error check
Metro throttleTimer = Metro(500); // Used for sending commands to Motor Controller
Metro tractiveTimeOut = Metro(5000); // Used to check timeout of tractive system activation

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
    set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
}

void loop() {
    while (CAN.read(msg)) {
        // TODO: Handle CAN messages from other components (e.g. MC, Dashboard)
        if (msg.id == ID_PCU_STATUS) {
            PCU_status pcu_status(msg.buf);
            if (pcu_status.get_bms_fault()) {
                Serial.println("BMS Fault detected");
            }
            if (pcu_status.get_imd_fault()) {
                Serial.println("IMD Fault detected");
            }
            switch (pcu_status.get_state()) {
                // NOTE: see if this should be happening (depending on current TCU state)
                case PCU_STATE_WAITING_BMS_IMD:
                    break;
                case PCU_STATE_WAITING_DRIVER:
                    break;
                case PCU_STATE_LATCHING:
                    set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
                    break;
                case PCU_STATE_FATAL_FAULT:
                    // assuming shutdown_circuit has opened
                    set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
                    break;
                case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
                    // TCU must wait until PCU in PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED to go into TCU_STATE_WAITING_TRACTIVE_SYSTEM
                    if (state == TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED || state == TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
                        set_state(TCU_STATE_WAITING_TRACTIVE_SYSTEM);
                    }
                    break;
            }
        } else if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
            MC_voltage_information mc_voltage_information(msg.buf);
            if (state == TCU_STATE_WAITING_TRACTIVE_SYSTEM) {
                // This code checks (when waiting for tractive system) if the tractive system has turned on
                if (mc_voltage_information.get_dc_bus_voltage() > 100) {
                    // Assume this condition is the way to check if tractive system is on
                    set_state(TCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
                    // NOTE: You must assume that for tractive system to turn on, the AIRs will be closed
                }
            }
        }
    }

    // CAN BUS
// DC Bus voltage (Motor controller) is higher than 100 (v??)

    if (updateTimer.check()) {
        readValues();
        updateTimer.reset();
    }
    checkDeactivateTractiveSystem();
    // TODO: deactivate tractive system if above returns true
    }
    if (CANUpdateTimer.check()){
        sendCANUpdate();
        CANUpdateTimer.reset();
    }
    switch(state) {
        //TODO: check if reqs are met to move to each state
        case TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED:
            break;
        case TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
            // TCU must wait until PCU in SHUTDOWN_CIRCUIT_INITIALIZED state
            // NOTE: Process handled in CAN message handler
            break;
        case TCU_STATE_WAITING_TRACTIVE_SYSTEM:
            // TODO: check if tractive system is active, & shutdown circuit closed
            // then change state to tractive system active
            // NOTE: Don't know why we need both this and the CAN check we have above
            if (tractiveTimeOut.check()) {
                // time out has occured, tractive system not active state
                set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
            } else {
                set_state(TCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
            }
            break;
        case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
            // TODO - make sure start button and brake pressed
            // REVIEW: TCU will check for brake and start button press immediately (no delay?)
            if (brakePedalActive) { // TODO: check Start button on dashboard - code has not been written yet
                set_state(TCU_STATE_ENABLING_INVERTER);
            }
            // NOTE: there is no timeout for the above state change
            break;
        case TCU_STATE_ENABLING_INVERTER:
            // TODO: next state if inverter enabled
            set_state(TCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
            break;
        case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
            // TODO: sound goes off
            // TODO: state change if sound finished
            set_state(TCU_STATE_READY_TO_DRIVE);
            break;
        case TCU_STATE_READY_TO_DRIVE:
            break;
    }
}

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
        throttleImplausibility = implausibilityTimer.check();
    } else if (voltageThrottlePedal1 < MIN_THROTTLE_1 || voltageThrottlePedal2 < MIN_THROTTLE_2) {
        // Checks for failure of position sensor wiring
        // Check for open circuit or short to ground
        throttleImplausibility = implausibilityTimer.check();
    } else if (voltageThrottlePedal1 > MAX_THROTTLE_1 || voltageThrottlePedal2 > MAX_THROTTLE_2) {
        // Check for short to power
        throttleImplausibility = implausibilityTimer.check();
    } else {
        // No throttle implausibility detected
        implausibilityTimer.reset();
    }

    // Check brake pedal sensor
    if (voltageBrakePedal > MAX_BRAKE || voltageBrakePedal < MIN_BRAKE) {
        brakeImplausibility = true;
    } else {
        // No brake implausibility detected
        brakeImplausibility = false;
    }

    // return true if implausibility passes time threshold
    if (throttleImplausibility || brakeImplausibility) {
        return true;
    } else {
        return false;
    }
}

int sendCANUpdate(){
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
        return; // don't do anything if same state
    } else if (new_state == TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED) {
        // setup() call
        state = TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED;
    } else {
        switch (state) {
            // TODO handle state transitions
            case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
                if (new_state == TCU_STATE_ENABLING_INVERTER) {
                    state = TCU_STATE_ENABLING_INVERTER;
                }
                break;
            default:
                uint8_t old_state = state;
                state = new_state;
                break;
        }
    }
    sendCANUpdate(); // New message sent since change of state
}
