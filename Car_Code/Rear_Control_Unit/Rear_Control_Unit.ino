/*
 * HyTech 2018 Vehicle Rear Control Unit
 * Monitor Shutdown Circuit initialization.
 * Control power to motor controller, fans, and pump.
 * Send wireless telemetry data via XBee.
 * Configured for Power Board rev5
 */
#include <FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>
#include <XBTools.h>

/*
 * Pin definitions
 */
#define COOL_RELAY_1 A9
#define COOL_RELAY_2 2
#define COOL_RELAY_3 A8
#define GPIO1 A4
#define GPIO2 5
#define SENSE_12VSUPPLY A3
#define SENSE_BMS A1
#define SENSE_IMD A0
#define SENSE_SHUTDOWN_OUT A2
#define SSR_BRAKE_LIGHT 12
#define SSR_INVERTER 6

#define XB Serial2

/*
 * Constant definitions
 */
#define BMS_HIGH 134 // ~3V on BMS_OK line
#define IMD_HIGH 134 // ~3V on OKHS line
#define SHUTDOWN_OUT_HIGH 350 // ~8V on SHUTDOWN_C line
#define XBEE_PKT_LEN 15

/*
 * Timers
 */
Metro timer_bms_print_fault = Metro(500);
Metro timer_debug_bms_status = Metro(200);
Metro timer_debug_bms_temperatures = Metro(2000);
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
Metro timer_detailed_voltages = Metro(1000);
Metro timer_imd_print_fault = Metro(500);
Metro timer_restart_inverter = Metro(500); // Allow the FCU to restart the inverter
Metro timer_status_send = Metro(100);
Metro timer_status_send_xbee = Metro(1000);

/*
 * Global variables
 */
RCU_status rcu_status;
BMS_detailed_voltages bms_detailed_voltages[8][3];

boolean bms_faulting = false;
boolean imd_faulting = false;

boolean inverter_restart = false; // True when restarting the inverter

FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
    pinMode(COOL_RELAY_1, OUTPUT);
    pinMode(COOL_RELAY_2, OUTPUT);
    pinMode(COOL_RELAY_3, OUTPUT);
    pinMode(SSR_BRAKE_LIGHT, OUTPUT);
    pinMode(SSR_INVERTER, OUTPUT);

    Serial.begin(115200);
    CAN.begin();
    XB.begin(115200);
    delay(100);
    Serial.println("CAN system, serial communication, and XBee initialized");

    digitalWrite(SSR_INVERTER, HIGH);
    digitalWrite(COOL_RELAY_1, HIGH);
    digitalWrite(COOL_RELAY_2, HIGH);
    digitalWrite(COOL_RELAY_3, HIGH);
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
            digitalWrite(SSR_BRAKE_LIGHT, fcu_status.get_brake_pedal_active());
        }
        if (msg.id == ID_RCU_RESTART_MC) { // Restart inverter when the FCU restarts
            if (millis() > 1000) { // Ignore restart messages when this microcontroller has also just booted up
                inverter_restart = true;
                digitalWrite(SSR_INVERTER, LOW);
                timer_restart_inverter.reset();
            }
        }
       /*if ((msg.id == ID_MC_TEMPERATURES_1 && timer_debug_rms_temperatures_1.check())
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
       }*/
       if (msg.id == ID_BMS_DETAILED_VOLTAGES) {
           BMS_detailed_voltages temp = BMS_detailed_voltages(msg.buf);
           bms_detailed_voltages[temp.get_ic_id()][temp.get_group_id()].load(msg.buf);
       }
       send_xbee();
    }

    if (timer_detailed_voltages.check()) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 3; j++) {
                Serial.print("IC ");
                Serial.print(i);
                Serial.print(" Group ");
                Serial.print(j);
                Serial.print(" V0: ");
                Serial.print(bms_detailed_voltages[i][j].get_voltage(0) / (double) 10000, 4);
                Serial.print(" V1: ");
                Serial.print(bms_detailed_voltages[i][j].get_voltage(1) / (double) 10000, 4);
                Serial.print(" V2: ");
                Serial.println(bms_detailed_voltages[i][j].get_voltage(2) / (double) 10000, 4);
            }
        }
        Serial.println();
    }

    /*
     * Send status over CAN and XBee
     */
    if (timer_status_send.check()) {
        rcu_status.write(msg.buf);
        msg.id = ID_RCU_STATUS;
        msg.len = sizeof(CAN_message_rcu_status_t);
        CAN.write(msg);
        //write_xbee_data();
    }

    if (timer_status_send_xbee.check()) {
        if (!rcu_status.get_bms_ok_high() && millis() > 3500) {
            XB.println("RCU BMS FAULT: detected");
        }
        if (!rcu_status.get_imd_okhs_high() && millis() > 3500) {
            XB.println("RCU IMD FAULT: detected");
        }
        XB.print("RCU STATE: ");
        XB.println(rcu_status.get_state());
    }

    /*
     * Finish restarting the inverter when timer expires
     */
    if (inverter_restart && timer_restart_inverter.check()) {
        inverter_restart = false;
        digitalWrite(SSR_INVERTER, HIGH);
    }

    /*
     * Check for BMS fault
     */
    if (analogRead(SENSE_BMS) > BMS_HIGH) {
        rcu_status.set_bms_ok_high(true);
    } else {
        rcu_status.set_bms_ok_high(false);
        if (timer_bms_print_fault.check()) {
            Serial.println("BMS fault detected");
        }
    }

    /*
     * Check for IMD fault
     */
    if (analogRead(SENSE_IMD) > IMD_HIGH) {
        rcu_status.set_imd_okhs_high(true);
    } else {
        rcu_status.set_imd_okhs_high(false);
        if (timer_imd_print_fault.check()) {
            Serial.println("IMD fault detected");
        }
    }

    /*
     * Measure SHUTDOWN_C to determine if BMS/IMD relays have latched
     */
    if (analogRead(SENSE_SHUTDOWN_OUT) > SHUTDOWN_OUT_HIGH) {
        rcu_status.set_bms_imd_latched(true);
        digitalWrite(COOL_RELAY_2, HIGH);
        digitalWrite(COOL_RELAY_3, HIGH);
    } else {
        rcu_status.set_bms_imd_latched(false);
        digitalWrite(COOL_RELAY_2, LOW);
        digitalWrite(COOL_RELAY_3, LOW);
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
    uint8_t xb_buf[XBEE_PKT_LEN];
    memcpy(xb_buf, &msg.id, sizeof(msg.id));        // msg id
    memcpy(xb_buf + sizeof(msg.id), &msg.len, sizeof(uint8_t));     // msg len
    memcpy(xb_buf + sizeof(msg.id) + sizeof(uint8_t), msg.buf, msg.len); // msg contents

    // calculate checksum
    uint16_t checksum = fletcher16(xb_buf, XBEE_PKT_LEN - 2);
    Serial.print("CHECKSUM: ");
    Serial.println(checksum, HEX);
    memcpy(&xb_buf[XBEE_PKT_LEN - 2], &checksum, sizeof(uint16_t));

    for (int i = 0; i < XBEE_PKT_LEN; i++) {
      Serial.print(xb_buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    uint8_t cobs_buf[2 + XBEE_PKT_LEN];
    cobs_encode(xb_buf, XBEE_PKT_LEN+1, cobs_buf);
    cobs_buf[XBEE_PKT_LEN+1] = 0x0;

    for (int i = 0; i < XBEE_PKT_LEN+2; i++) {
      Serial.print(cobs_buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    int written = XB.write(cobs_buf, 2 + XBEE_PKT_LEN);
    Serial.print("Wrote ");
    Serial.print(written);
    Serial.println(" bytes");

    return written;
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
            XB.println(bms_voltages.get_average() / (double) 10000, 4);
            XB.print("BMS VOLTAGE LOW: ");
            XB.println(bms_voltages.get_low() / (double) 10000, 4);
            XB.print("BMS VOLTAGE HIGH: ");
            XB.println(bms_voltages.get_high() / (double) 10000, 4);
            XB.print("BMS VOLTAGE TOTAL: ");
            XB.println(bms_voltages.get_total() / (double) 100, 2);
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
