/*
 * HyTech 2017 Vehicle Rear Control Unit
 * Init 2017-06-02
 * Control Shutdown Circuit initialization.
 * Configured for Power Board rev3
 */
#include <FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>
#include <XBTools.h>

/*
 * Pin definitions
 */
#define COOL_MOSFET_1 A8
#define COOL_MOSFET_2 A7
#define COOL_MOSFET_3 A6
#define COOL_RELAY_1 A9
#define COOL_RELAY_2 2
#define SENSE_BMS A1
#define SENSE_IMD A0
#define SENSE_SHUTDOWN_OUT A2
#define SSR_BRAKE_LIGHT 12
#define SSR_INVERTER 6
#define SSR_LATCH_BMS 11
#define SSR_LATCH_IMD 8

#define XB Serial2

/*
 * Constant definitions
 */
#define BMS_HIGH 100
#define BMS_LOW 50
#define IMD_HIGH 100
#define IMD_LOW 50

/*
 * Timers
 */
Metro timer_bms_faulting = Metro(1000); // At startup the BMS_OK line drops shortly
Metro timer_debug_bms_status = Metro(1000);
Metro timer_debug_bms_temperatures = Metro(1000);
Metro timer_debug_bms_voltages = Metro(1000);
Metro timer_debug_rms_current_information = Metro(2000);
Metro timer_debug_rms_fault_codes = Metro(2000);
Metro timer_debug_rms_internal_states = Metro(2000);
Metro timer_debug_rms_motor_position_information = Metro(2000);
Metro timer_debug_rms_temperatures_1 = Metro(2000);
Metro timer_debug_rms_temperatures_3 = Metro(2000);
Metro timer_debug_rms_torque_timer_information = Metro(2000);
Metro timer_debug_rms_voltage_information = Metro(2000);
Metro timer_debug_fcu_status = Metro(2000);
Metro timer_imd_faulting = Metro(1000); // At startup the IMD_OKHS line drops shortly
Metro timer_latch = Metro(1000);
Metro timer_state_send = Metro(100);
Metro timer_fcu_restart_inverter = Metro(500); // Upon restart of the FCU, power cycle the inverter

/*
 * Global variables
 */
RCU_status rcu_status;

boolean bms_faulting = false;
uint8_t btn_start_id = 0; // increments to differentiate separate button presses
uint8_t btn_start_new = 0;
boolean imd_faulting = false;

FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
    pinMode(COOL_RELAY_1, OUTPUT);
    pinMode(COOL_RELAY_2, OUTPUT);
    pinMode(SSR_BRAKE_LIGHT, OUTPUT);
    pinMode(SSR_INVERTER, OUTPUT);
    pinMode(SSR_LATCH_BMS, OUTPUT);
    pinMode(SSR_LATCH_IMD, OUTPUT);

    Serial.begin(115200);
    CAN.begin();
    XB.begin(115200);
    delay(100);
    Serial.println("CAN system, serial communication, and XBee initialized");

    digitalWrite(SSR_INVERTER, HIGH);
    digitalWrite(COOL_RELAY_1, HIGH);
    digitalWrite(COOL_RELAY_2, HIGH);
    set_state(RCU_STATE_WAITING_BMS_IMD);
    rcu_status.set_bms_ok_high(true);
    rcu_status.set_imd_okhs_high(true);
}

void loop() {
    /*
     * Handle incoming CAN messages
     */
    while (CAN.read(msg)) {
        if (msg.id == ID_FCU_STATUS) {
            FCU_status fcu_status = FCU_status(msg.buf);
            if (fcu_status.get_brake_pedal_active()) {
                digitalWrite(SSR_BRAKE_LIGHT, HIGH);
            }
            else {
                digitalWrite(SSR_BRAKE_LIGHT, LOW);
            }
            if (btn_start_id != fcu_status.get_start_button_press_id()) {
                btn_start_id = fcu_status.get_start_button_press_id();
                Serial.print("Start button pressed id ");
                Serial.println(btn_start_id);
            }
        }
        if (msg.id == ID_FCU_RESTART) {
            if (millis() > 1000) { // Ignore restart messages when this microcontroller has also just booted up
                digitalWrite(SSR_INVERTER, LOW);
                timer_fcu_restart_inverter.reset();
                set_state(0);
            }
        }
        if ((msg.id == ID_MC_TEMPERATURES_1 && timer_debug_rms_temperatures_1.check())
                || (msg.id == ID_MC_TEMPERATURES_3 && timer_debug_rms_temperatures_3.check())
                || (msg.id == ID_MC_MOTOR_POSITION_INFORMATION && timer_debug_rms_motor_position_information.check())
                || (msg.id == ID_MC_CURRENT_INFORMATION && timer_debug_rms_current_information.check())
                || (msg.id == ID_MC_VOLTAGE_INFORMATION && timer_debug_rms_voltage_information.check())
                || (msg.id == ID_MC_INTERNAL_STATES && timer_debug_rms_internal_states.check())
                || (msg.id == ID_MC_FAULT_CODES && timer_debug_rms_fault_codes.check())
                || (msg.id == ID_MC_TORQUE_TIMER_INFORMATION && timer_debug_rms_torque_timer_information.check())
                || (msg.id == ID_BMS_VOLTAGES && timer_debug_bms_voltages.check())
                || (msg.id == ID_BMS_TEMPERATURES && timer_debug_bms_temperatures.check())
                || (msg.id == ID_BMS_STATUS && timer_debug_bms_status.check())
                || (msg.id == ID_FCU_STATUS && timer_debug_fcu_status.check())) {
            write_xbee_data();
        }
    }

    /*
     * Send state over CAN and XBee
     */
    if (timer_state_send.check()) {
        rcu_status.write(msg.buf);
        msg.id = ID_RCU_STATUS;
        msg.len = sizeof(CAN_message_rcu_status_t);
        CAN.write(msg);

        write_xbee_data();

        // commented below code in case of problems
        
        // if (!rcu_status.get_bms_ok_high()) { // TODO make sure this doesn't happen at startup
        //     XB.println("RCU BMS FAULT: detected");
        // }
        // if (!rcu_status.get_imd_okhs_high()) { // TODO make sure this doesn't happen at startup
        //     XB.println("RCU IMD FAULT: detected");
        // }
        // XB.print("RCU STATE: ");
        // XB.println(rcu_status.get_state());
    }

    /*
     * State machine
     */
    switch (rcu_status.get_state()) {
        case 0:
        if (timer_fcu_restart_inverter.check()) {
            digitalWrite(SSR_INVERTER, HIGH);
            set_state(RCU_STATE_WAITING_BMS_IMD);
        }
        break;
            
        case RCU_STATE_WAITING_BMS_IMD:
        if (analogRead(SENSE_IMD) > IMD_HIGH && analogRead(SENSE_BMS) > BMS_HIGH) { // Wait till IMD and BMS signals go high at startup
            set_state(RCU_STATE_WAITING_DRIVER);
        }
        break;

        case RCU_STATE_WAITING_DRIVER:
        if (btn_start_new == btn_start_id) { // Start button has been pressed
            set_state(RCU_STATE_LATCHING);
        }
        break;

        case RCU_STATE_LATCHING:
        if (timer_latch.check()) { // Disable latching SSR
            set_state(RCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED);
        }
        break;

        case RCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
        break;

        case RCU_STATE_FATAL_FAULT:
        break;
    }

    /*
     * Start BMS fault timer if signal drops momentarily
     */
    if (rcu_status.get_state() != RCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_BMS) <= BMS_LOW) { // TODO imd/bms
        bms_faulting = true;
        timer_bms_faulting.reset();
    }

    /*
     * Reset BMS fault condition if signal comes back within timer period
     */
    if (bms_faulting && analogRead(SENSE_BMS) > BMS_HIGH) {
        bms_faulting = false;
    }

    /*
     * Declare BMS fault if signal still dropped
     */
    if (bms_faulting && timer_bms_faulting.check()) {
        rcu_status.set_bms_ok_high(false);
        set_state(RCU_STATE_FATAL_FAULT);
        Serial.println("BMS fault detected");
    }

    /*
     * Start IMD fault timer if signal drops momentarily
     */
    if (rcu_status.get_state() != RCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_IMD) <= IMD_LOW) {
        imd_faulting = true;
        timer_imd_faulting.reset();
    }

    /*
     * Reset IMD fault condition if signal comes back within timer period
     */
    if (imd_faulting && analogRead(SENSE_IMD) > IMD_HIGH) {
        imd_faulting = false;
    }

    /*
     * Declare IMD fault if signal still dropped
     */
    if (imd_faulting && timer_imd_faulting.check()) {
        rcu_status.set_imd_okhs_high(false);
        set_state(RCU_STATE_FATAL_FAULT);
        Serial.println("IMD fault detected");
    }
}

/*
 * Handle changes in state
 */
void set_state(uint8_t new_state) {
    if (rcu_status.get_state() == new_state) {
        return;
    }
    rcu_status.set_state(new_state);
    if (new_state == RCU_STATE_WAITING_DRIVER) {
        btn_start_new = btn_start_id + 1;
    }
    if (new_state == RCU_STATE_LATCHING) {
        timer_latch.reset();
        digitalWrite(SSR_LATCH_BMS, HIGH);
        digitalWrite(SSR_LATCH_IMD, HIGH);
        Serial.println("Latching");
    }
    if (new_state == RCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED) {
        digitalWrite(SSR_LATCH_BMS, LOW);
        digitalWrite(SSR_LATCH_IMD, LOW);
        digitalWrite(COOL_RELAY_2, HIGH);
    }
}

/**
 * Writes data currently in global msg variable to the Xbee serial bus.
 * Calculates Fletcher checksum and byte-stuffs so that messages are
 * delimited by 0x0 bytes.
 * 
 * returns: number of bytes written to the Xbee serial bus
 */
int write_xbee_data() {
    // delim (1) + checksum (2) + id (4) + length (1) + length
    uint8_t state_msg_size = sizeof(uint16_t) + sizeof(msg.id) + sizeof(uint8_t) + msg.len;
    uint8_t xb_buf[state_msg_size];
    memcpy(xb_buf, &msg.id, sizeof(msg.id));        // msg id
    memcpy(xb_buf + sizeof(msg.id), &msg.len, sizeof(uint8_t));     // msg len
    memcpy(xb_buf + sizeof(msg.id) + sizeof(uint8_t), msg.buf, msg.len); // msg contents

    // calculate checksum
    uint16_t checksum = fletcher16(xb_buf, sizeof(msg.id) + sizeof(uint8_t) + msg.len);
    memcpy(xb_buf + sizeof(msg.id) + sizeof(uint8_t) + msg.len, &checksum, sizeof(uint16_t));

    uint8_t cobs_buf[1 + state_msg_size];
    cobs_encode(xb_buf, state_msg_size, cobs_buf);
    cobs_buf[state_msg_size] = 0x0;

    return XB.write(cobs_buf, 1 + state_msg_size);
}