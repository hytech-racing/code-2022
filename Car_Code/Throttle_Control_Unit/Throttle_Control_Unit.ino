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
                        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
                        break;
                    case PCU_STATE_WAITING_DRIVER:
                        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
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
                        if (state == TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED) {
                            set_state(TCU_STATE_WAITING_TRACTIVE_SYSTEM);
                        }
                }
            }
        }

        if (updateTimer.check()) {
            readValues();
            updateTimer.reset();
        }
        if (implausibilityTimer.check()) {
            checkDeactivateTractiveSystem();
            // TODO: deactivate tractive system if above returns true
            implausibilityTimer.reset();
        }
        if(stateTimer.check()){
            sendCanUpdate();
            stateTimer.reset();
        }
        switch(state) {
            //TODO: check if reqs are met to move to each state
            case TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED:
                // TCU must wait until PCU in SHUTDOWN_CIRCUIT_INITIALIZED state
                if (pcu_status.get_state() == PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED) {
                    state = TCU_STATE_WAITING_TRACTIVE_SYSTEM;
                    tractiveTimeOut.reset(); // resets to check if time out
                }
                break;
            case TCU_STATE_WAITING_TRACTIVE_SYSTEM:
                // TODO: check if tractive system is active, & shutdown circuit closed
                // then change state to tractive system active
                if (tractiveTimeOut.check()) {
                    // time out has occured, tractive system not active state
                    state = TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE;
                }
                state = TCU_STATE_TRACTIVE_SYSTEM_ACTIVE;
                break;
            case TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
                // TODO
                state = TCU_STATE_TRACTIVE_SYSTEM_ACTIVE;
                break;
            case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
                // TODO - make sure start button and brake pressed
                // REVIEW: TCU will check for brake and start button press immediately (no delay?)
                if (brakePedalActive) { // TODO: check Start button on dashboard - code has not been written yet
                    set_state(TCU_STATE_ENABLING_INVERTER)
                }
                // NOTE: there is no timeout for the above state change
                break;
            case TCU_STATE_ENABLING_INVERTER:
                // TODO
                state = TCU_STATE_WAITING_READY_TO_DRIVE_SOUND;
                break;
            case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
                // TODO
                state = TCU_STATE_READY_TO_DRIVE;
                break;
            case TCU_STATE_READY_TO_DRIVE:
                // TODO
                break;
        }
    }
    // Steps of State machine:
        // Wait for the Power Control Unit to send a CAN Bus message showing that it has entered the Shutdown Circuit Initialized state
        // Check if the Tractive System is really active. There will be a periodic message on the CAN Bus indicating if the Tractive System is active. This message may originate from the motor controller, or a HyTech ECU. When the Tractive System is active, proceed to the Tractive System Active state. If the Tractive System never activates or becomes inactive, the Throttle Control Unit should enter the Tractive System Not Active state.
        // The driver must press the Start Button on the dashboard, and press the brake pedal simultaneously to proceed. Wait for a CAN Bus message showing the Start Button is pressed, along with detecting that the brake pedal is simultaneously pressed. The brake pedal threshold should be the same required to light up the brake lights.
        // Send the Inverter Enable command to the Rinehart PM100DX motor controller. Then wait for the motor controller to return a message that the inverter is enabled.
        // If the inverter successfully enables, send a Ready to Drive Sound (RTDS) Enable message on CAN Bus for the Dashboard Control Unit to read.
        // The Dashboard Control Unit will read the RTDS Enable message and sound the RTDS. After the RTDS, the Dashboard ECU will send a Ready to Drive Sound Finished message onto CAN Bus. The Throttle Control Unit should wait for this message, then enter Ready to Drive state, sending torque commands to the motor controller.
        // If after timeout periods the inverter does not successfully enable or RTDS does not sound, send an inverter disable message then proceed back to step 3 and wait for the driver to press the start button and brake pedal (TODO maybe this should declare a fault; any thoughts about sending inverter disable message?)
        // If there is a Software Fault, BMS Fault, or IMD Fault, the Power Control Unit will enter a Fatal Fault state. Since the tractive system will be disabled, the Throttle Control Unit should enter the Tractive System Not Active State outlined in Step 2. (TODO maybe revert this to entering a fatal fault state)

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

    // The Throttle Control Unit should periodically send out messages on CAN Bus detailing its current state including:
        // Raw throttle input values
        // Implausibility status
        // Throttle curve in use (normal or boost)
        // Temperature as read by onboard thermistor
        // Brake System Plausibility Device status (boolean indicating whether a fault has occurred)
        // Brake Pedal Active (boolean used by Power Control Unit to control brake lights)
    // The following Inverter Initialization states should be sent in a CAN Bus message periodically as well as immediately upon change of state.
        // Waiting for Shutdown Circuit Initialization
        // Waiting for Tractive System Active
        // Waiting for driver (start button and brake pedal haven’t been pressed)
        // Enabling inverter (sent enable command, waiting for Ready to Drive Sound)
        // Ready to Drive (vehicle responds to throttle)

}

void set_state(uint8_t new_state) {
    if (state == new_state) {
        return; // don't do anything if same state
    }
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
