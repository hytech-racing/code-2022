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
        send_xbee();
    }

    /*
     * Send state over CAN and XBee
     */
    if (timer_state_send.check()) {
        rcu_status.write(msg.buf);
        msg.id = ID_RCU_STATUS;
        msg.len = sizeof(CAN_message_rcu_status_t);
        CAN.write(msg);

        // delim (1) + checksum (2) + id (id-size) + length (1) + length
        uint8_t state_msg_size = sizeof(uint16_t) + sizeof(msg.id) + sizeof(uint8_t) + msg.len;
        uint8_t xb_buf[state_msg_size];
        memcpy(xb_buf, &msg.id, sizeof(msg.id));        // msg id
        memcpy(xb_buf + sizeof(msg.id), &msg.len, sizeof(uint8_t));     // msg len
        memcpy(xb_buf + sizeof(msg.id) + sizeof(uint8_t), msg.buf, msg.len); // msg contents

        // calculate checksum
        uint16_t checksum = fletcher16(xb_buf, sizeof(msg.id) + sizeof(uint8_t) + msg.len);
        memcpy(xb_buf + sizeof(msg.id) + sizeof(uint8_t) + msg.len, &checksum, sizeof(uint16_t));

        uint8_t cobs_buf[1 + state_msg_size];
        cobs_encode(xb_buf, state_msg_size, cobs_buf + 1);
        cobs_buf[0] = 0x0;

        XB.write(cobs_buf, 1 + state_msg_size);

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

void send_xbee() {
    if (msg.id == ID_MC_TEMPERATURES_1 && timer_debug_rms_temperatures_1.check()) {
            MC_temperatures_1 mc_temperatures_1 = MC_temperatures_1(msg.buf);
            XB.print("MODULE A TEMP: ");
            XB.println(mc_temperatures_1.get_module_a_temperature() / (double) 10, 1);
            XB.print("MODULE B TEMP: ");
            XB.println(mc_temperatures_1.get_module_b_temperature() / (double) 10, 1);
            XB.print("MODULE C TEMP: ");
            XB.println(mc_temperatures_1.get_module_c_temperature() / (double) 10, 1);
            XB.print("GATE DRIVER BOARD TEMP: ");
            XB.println(mc_temperatures_1.get_gate_driver_board_temperature() / (double) 10, 1);
        }

        if (msg.id ==ID_MC_TEMPERATURES_3 && timer_debug_rms_temperatures_3.check()) {
            MC_temperatures_3 mc_temperatures_3 = MC_temperatures_3(msg.buf);
            //XB.print("RTD 4 TEMP: "); // These aren't needed since we aren't using RTDs
            //XB.println(mc_temperatures_3.get_rtd_4_temperature());
            //XB.print("RTD 5 TEMP: ");
            //XB.println(mc_temperatures_3.get_rtd_5_temperature());
            XB.print("MOTOR TEMP: ");
            XB.println(mc_temperatures_3.get_motor_temperature() / (double) 10, 1);
            XB.print("TORQUE SHUDDER: ");
            XB.println(mc_temperatures_3.get_torque_shudder() / (double) 10, 1);
        }

        if (msg.id == ID_MC_MOTOR_POSITION_INFORMATION && timer_debug_rms_motor_position_information.check()) {
            MC_motor_position_information mc_motor_position_information = MC_motor_position_information(msg.buf);
            XB.print("MOTOR ANGLE: ");
            XB.println(mc_motor_position_information.get_motor_angle());
            XB.print("MOTOR SPEED: ");
            XB.println(mc_motor_position_information.get_motor_speed());
            XB.print("ELEC OUTPUT FREQ: ");
            XB.println(mc_motor_position_information.get_electrical_output_frequency());
            XB.print("DELTA RESOLVER FILT: ");
            XB.println(mc_motor_position_information.get_delta_resolver_filtered());
        }

        if (msg.id == ID_MC_CURRENT_INFORMATION && timer_debug_rms_current_information.check()) {
            MC_current_information mc_current_information = MC_current_information(msg.buf);
            XB.print("PHASE A CURRENT: ");
            XB.println(mc_current_information.get_phase_a_current() / (double) 10, 1);
            XB.print("PHASE B CURRENT: ");
            XB.println(mc_current_information.get_phase_b_current() / (double) 10, 1);
            XB.print("PHASE C CURRENT: ");
            XB.println(mc_current_information.get_phase_c_current() / (double) 10, 1);
            XB.print("DC BUS CURRENT: ");
            XB.println(mc_current_information.get_dc_bus_current() / (double) 10, 1);
        }

        if (msg.id == ID_MC_VOLTAGE_INFORMATION && timer_debug_rms_voltage_information.check()) {
            MC_voltage_information mc_voltage_information = MC_voltage_information(msg.buf);
            XB.print("DC BUS VOLTAGE: ");
            XB.println(mc_voltage_information.get_dc_bus_voltage() / (double) 10, 1);
            XB.print("OUTPUT VOLTAGE: ");
            XB.println(mc_voltage_information.get_output_voltage() / (double) 10, 1);
            XB.print("PHASE AB VOLTAGE: ");
            XB.println(mc_voltage_information.get_phase_ab_voltage() / (double) 10, 1);
            XB.print("PHASE BC VOLTAGE: ");
            XB.println(mc_voltage_information.get_phase_bc_voltage() / (double) 10, 1);
        }

        if (msg.id == ID_MC_INTERNAL_STATES && timer_debug_rms_internal_states.check()) {
            MC_internal_states mc_internal_states = MC_internal_states(msg.buf);
            XB.print("VSM STATE: ");
            XB.println(mc_internal_states.get_vsm_state());
            XB.print("INVERTER STATE: ");
            XB.println(mc_internal_states.get_inverter_state());
            XB.print("INVERTER RUN MODE: ");
            XB.println(mc_internal_states.get_inverter_run_mode());
            XB.print("INVERTER ACTIVE DISCHARGE STATE: ");
            XB.println(mc_internal_states.get_inverter_active_discharge_state());
            XB.print("INVERTER COMMAND MODE: ");
            XB.println(mc_internal_states.get_inverter_command_mode());
            XB.print("INVERTER ENABLE: ");
            XB.println(mc_internal_states.get_inverter_enable_state());
            XB.print("INVERTER LOCKOUT: ");
            XB.println(mc_internal_states.get_inverter_enable_lockout());
            XB.print("DIRECTION COMMAND: ");
            XB.println(mc_internal_states.get_direction_command());
        }

        if (msg.id == ID_MC_FAULT_CODES && timer_debug_rms_fault_codes.check()) {
            MC_fault_codes mc_fault_codes = MC_fault_codes(msg.buf);
            XB.print("POST FAULT LO: 0x");
            XB.println(mc_fault_codes.get_post_fault_lo(), HEX);
            XB.print("POST FAULT HI: 0x");
            XB.println(mc_fault_codes.get_post_fault_hi(), HEX);
            XB.print("RUN FAULT LO: 0x");
            XB.println(mc_fault_codes.get_run_fault_lo(), HEX);
            XB.print("RUN FAULT HI: 0x");
            XB.println(mc_fault_codes.get_run_fault_hi(), HEX);
        }

        if (msg.id == ID_MC_TORQUE_TIMER_INFORMATION && timer_debug_rms_torque_timer_information.check()) {
            MC_torque_timer_information mc_torque_timer_information = MC_torque_timer_information(msg.buf);
            XB.print("COMMANDED TORQUE: ");
            XB.println(mc_torque_timer_information.get_commanded_torque() / (double) 10, 1);
            XB.print("TORQUE FEEDBACK: ");
            XB.println(mc_torque_timer_information.get_torque_feedback());
            XB.print("RMS UPTIME: ");
            XB.println(mc_torque_timer_information.get_power_on_timer() * .003, 0);
        }

        if (msg.id == ID_BMS_VOLTAGES && timer_debug_bms_voltages.check()) {
            BMS_voltages bms_voltages = BMS_voltages(msg.buf);
            XB.print("BMS VOLTAGE AVERAGE: ");
            XB.println(bms_voltages.get_average() / (double) 1000, 4);
            XB.print("BMS VOLTAGE LOW: ");
            XB.println(bms_voltages.get_low() / (double) 10000, 4);
            XB.print("BMS VOLTAGE HIGH: ");
            XB.println(bms_voltages.get_high() / (double) 10000, 4);
            XB.print("BMS VOLTAGE TOTAL: ");
            XB.println(bms_voltages.get_total());
        }
        
        if (msg.id == ID_BMS_TEMPERATURES && timer_debug_bms_temperatures.check()) {
            BMS_temperatures bms_temperatures = BMS_temperatures(msg.buf);
            XB.print("BMS AVERAGE TEMPERATURE: ");
            XB.println(bms_temperatures.get_average_temperature() / (double) 100, 2);
            XB.print("BMS LOW TEMPERATURE: ");
            XB.println(bms_temperatures.get_low_temperature() / (double) 100, 2);
            XB.print("BMS HIGH TEMPERATURE: ");
            XB.println(bms_temperatures.get_high_temperature() / (double) 100, 2);
        }

        if (msg.id == ID_BMS_STATUS && timer_debug_bms_status.check()) {
            BMS_status bms_status = BMS_status(msg.buf);
            XB.print("BMS STATE: ");
            XB.println(bms_status.get_state());
            XB.print("BMS ERROR FLAGS: 0x");
            XB.println(bms_status.get_error_flags(), HEX);
            XB.print("BMS CURRENT: ");
            XB.println(bms_status.get_current() / (double) 100, 2);
        }

        if (msg.id == ID_FCU_STATUS && timer_debug_fcu_status.check()) {
            FCU_status fcu_status = FCU_status(msg.buf);
            XB.print("FCU BRAKE ACT: ");
            XB.println(fcu_status.get_brake_pedal_active());
            XB.print("FCU STATE: ");
            XB.println(fcu_status.get_state());
        }

        /*if (msg.id == ID_MC_COMMAND_MESSAGE) { // TODO bring this code up to date with the debug.py system
        MC_command_message mc_command_message = MC_command_message(msg.buf);
        Serial.print("Torque command: ");
        Serial.println(mc_command_message.get_torque_command());
        Serial.print("Angular velocity: ");
        Serial.println(mc_command_message.get_angular_velocity());
        Serial.print("Direction: ");
        Serial.println(mc_command_message.get_direction());
        Serial.print("Inverter enable: ");
        Serial.println(mc_command_message.get_inverter_enable());
        Serial.print("Discharge enable: ");
        Serial.println(mc_command_message.get_discharge_enable());
        Serial.print("Commanded torque limit: ");
        Serial.println(mc_command_message.get_commanded_torque_limit());
        }*/
}