/*
 * HyTech 2017 Vehicle Rear Control Unit
 * Init 2017-06-02
 * Control Shutdown Circuit initialization.
 * Configured for Power Board rev3
 */
#include <FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>

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
Metro timer_bms_faulting = Metro(1000); // At startup the BMS DISCHARGE_OK line drops shortly
Metro timer_debug_bms_status = Metro(2000);
Metro timer_debug_bms_temperatures = Metro(2000);
Metro timer_debug_bms_voltages = Metro(2000);
Metro timer_debug_rms_current_information = Metro(2000);
Metro timer_debug_rms_fault_codes = Metro(2000);
Metro timer_debug_rms_internal_states = Metro(2000);
Metro timer_debug_rms_motor_position_information = Metro(2000);
Metro timer_debug_rms_temperatures_1 = Metro(2000);
Metro timer_debug_rms_temperatures_3 = Metro(2000);
Metro timer_debug_rms_torque_timer_information = Metro(2000);
Metro timer_debug_rms_voltage_information = Metro(2000);
Metro timer_debug_tcu_status = Metro(2000);
Metro timer_imd_faulting = Metro(1000); // At startup the IMD OKHS line drops shortly
Metro timer_latch = Metro(1000);
Metro timer_state_send = Metro(100);
Metro timer_tcu_restart_inverter = Metro(500); // Upon restart of the TCU, power cycle the inverter
Metro timer_xbee_send = Metro(1000);

/*
 * Global variables
 */
boolean bms_fault = false;
boolean bms_faulting = false;
uint8_t btn_start_id = 0; // increments to differentiate separate button presses
uint8_t btn_start_new = 0;
boolean imd_fault = false;
boolean imd_faulting = false;
uint8_t state = PCU_STATE_WAITING_BMS_IMD;

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
}

void loop() {
    /*
     * Handle incoming CAN messages
     */
    while (CAN.read(msg)) {
        if (msg.id == ID_DCU_STATUS) {
            DCU_status message = DCU_status(msg.buf);
            if (btn_start_id != message.get_btn_press_id()) {
                btn_start_id = message.get_btn_press_id();
                Serial.print("Start button pressed id ");
                Serial.println(btn_start_id);
            }
        }
        if (msg.id == ID_TCU_STATUS) {
            TCU_status tcu_status = TCU_status(msg.buf);
            if (tcu_status.get_brake_pedal_active()) {
                digitalWrite(SSR_BRAKE_LIGHT, HIGH);
            }
            else {
                digitalWrite(SSR_BRAKE_LIGHT, LOW);
            }
        }
        if (msg.id == ID_TCU_RESTART) {
            if (millis() > 1000) { // Ignore restart messages when this microcontroller has also just booted up
                digitalWrite(SSR_INVERTER, LOW);
                timer_tcu_restart_inverter.reset();
                set_state(0);
            }
        }
        /*if (msg.id == ID_MC_COMMAND_MESSAGE) {
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
        send_xbee();
    }

    /*
     * Send state over CAN and XBee
     */
    if (timer_state_send.check()) {
        PCU_status pcu_status(state, bms_fault, imd_fault, 0, 0); // Nothing external relies on OKHS or discharge_ok voltage so sending 0s for now
        pcu_status.write(msg.buf);
        msg.id = ID_PCU_STATUS;
        msg.len = sizeof(CAN_message_pcu_status_t);
        CAN.write(msg);

        if (pcu_status.get_bms_fault()) {
            XB.println("RCU BMS FAULT: detected");
        }
        if (pcu_status.get_imd_fault()) {
            XB.println("RCU IMD FAULT: detected");
        }
        XB.print("RCU STATE: ");
        XB.println(pcu_status.get_state());
    }

    switch (state) {
        case 0:
        if (timer_tcu_restart_inverter.check()) {
            digitalWrite(SSR_INVERTER, HIGH);
            set_state(PCU_STATE_WAITING_BMS_IMD);
        }
        break;
            
        case PCU_STATE_WAITING_BMS_IMD:
        if (analogRead(SENSE_IMD) > IMD_HIGH && analogRead(SENSE_BMS) > BMS_HIGH) { // Wait till IMD and BMS signals go high at startup
            set_state(PCU_STATE_WAITING_DRIVER);
        }
        break;

        case PCU_STATE_WAITING_DRIVER:
        if (btn_start_new == btn_start_id) { // Start button has been pressed
            set_state(PCU_STATE_LATCHING);
        }
        break;

        case PCU_STATE_LATCHING:
        if (timer_latch.check()) { // Disable latching SSR
            set_state(PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED);
        }
        break;

        case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
        break;

        case PCU_STATE_FATAL_FAULT:
        break;
    }

    /*
     * Start BMS fault timer if signal drops momentarily
     */
    if (state != PCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_BMS) <= BMS_LOW) { // TODO imd/bms
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
        bms_fault = true;
        set_state(PCU_STATE_FATAL_FAULT);
        Serial.println("BMS fault detected");
    }

    /*
     * Start IMD fault timer if signal drops momentarily
     */
    if (state != PCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_IMD) <= IMD_LOW) {
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
        imd_fault = true;
        set_state(PCU_STATE_FATAL_FAULT);
        Serial.println("IMD fault detected");
    }
}

/*
 * Handle changes in state
 */
void set_state(uint8_t new_state) {
    if (state == new_state) {
        return;
    }
    state = new_state;
    if (new_state == PCU_STATE_WAITING_DRIVER) {
        btn_start_new = btn_start_id + 1;
    }
    if (new_state == PCU_STATE_LATCHING) {
        timer_latch.reset();
        digitalWrite(SSR_LATCH_BMS, HIGH);
        digitalWrite(SSR_LATCH_IMD, HIGH);
        Serial.println("Latching");
    }
    if (new_state == PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED) {
        digitalWrite(SSR_LATCH_BMS, LOW);
        digitalWrite(SSR_LATCH_IMD, LOW);
        digitalWrite(COOL_RELAY_2, HIGH);
    }
}

void send_xbee() {
    if (msg.id == ID_MC_TEMPERATURES_1 && timer_debug_rms_temperatures_1.check()) {
            MC_temperatures_1 mc_temperatures_1 = MC_temperatures_1(msg.buf);
            XB.print("MODULE A TEMP: ");
            XB.println(mc_temperatures_1.get_module_a_temperature());
            XB.print("MODULE B TEMP: ");
            XB.println(mc_temperatures_1.get_module_b_temperature());
            XB.print("MODULE C TEMP: ");
            XB.println(mc_temperatures_1.get_module_c_temperature());
            XB.print("GATE DRIVER BOARD TEMP: ");
            XB.println(mc_temperatures_1.get_gate_driver_board_temperature());
        }

        if (msg.id ==ID_MC_TEMPERATURES_3 && timer_debug_rms_temperatures_3.check()) {
            MC_temperatures_3 mc_temperatures_3 = MC_temperatures_3(msg.buf);
            XB.print("RTD 4 TEMP: ");
            XB.println(mc_temperatures_3.get_rtd_4_temperature());
            XB.print("RTD 5 TEMP: ");
            XB.println(mc_temperatures_3.get_rtd_5_temperature());
            XB.print("MOTOR TEMP: ");
            XB.println(mc_temperatures_3.get_motor_temperature());
            XB.print("TORQUE SHUDDER: ");
            XB.println(mc_temperatures_3.get_torque_shudder());
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
            XB.println(mc_current_information.get_phase_a_current());
            XB.print("PHASE B CURRENT: ");
            XB.println(mc_current_information.get_phase_b_current());
            XB.print("PHASE C CURRENT: ");
            XB.println(mc_current_information.get_phase_c_current());
            XB.print("DC BUS CURRENT: ");
            XB.println(mc_current_information.get_dc_bus_current());
        }

        if (msg.id == ID_MC_VOLTAGE_INFORMATION && timer_debug_rms_voltage_information.check()) {
            MC_voltage_information mc_voltage_information = MC_voltage_information(msg.buf);
            XB.print("DC BUS VOLTAGE: ");
            XB.println(mc_voltage_information.get_dc_bus_voltage());
            XB.print("OUTPUT VOLTAGE: ");
            XB.println(mc_voltage_information.get_output_voltage());
            XB.print("PHASE AB VOLTAGE: ");
            XB.println(mc_voltage_information.get_phase_ab_voltage());
            XB.print("PHASE BC VOLTAGE: ");
            XB.println(mc_voltage_information.get_phase_bc_voltage());
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
            XB.print("POST FAULT LO: ");
            XB.println(mc_fault_codes.get_post_fault_lo());
            XB.print("POST FAULT HI: ");
            XB.println(mc_fault_codes.get_post_fault_hi());
            XB.print("RUN FAULT LO: ");
            XB.println(mc_fault_codes.get_run_fault_lo());
            XB.print("RUN FAULT HI: ");
            XB.println(mc_fault_codes.get_run_fault_hi());
        }

        if (msg.id == ID_MC_TORQUE_TIMER_INFORMATION && timer_debug_rms_torque_timer_information.check()) {
            MC_torque_timer_information mc_torque_timer_information = MC_torque_timer_information(msg.buf);
            XB.print("COMMANDED TORQUE: ");
            XB.println(mc_torque_timer_information.get_commanded_torque());
            XB.print("TORQUE FEEDBACK: ");
            XB.println(mc_torque_timer_information.get_torque_feedback());
            XB.print("RMS UPTIME: ");
            XB.println(mc_torque_timer_information.get_power_on_timer());
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

        if (msg.id == ID_TCU_STATUS && timer_debug_tcu_status.check()) {
            TCU_status tcu_status = TCU_status(msg.buf);
            XB.print("FCU BRAKE ACT: ");
            XB.println(tcu_status.get_brake_pedal_active());
            XB.print("FCU STATE: ");
            XB.println(tcu_status.get_state());
        }
}