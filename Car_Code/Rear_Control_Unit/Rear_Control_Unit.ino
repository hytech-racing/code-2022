/*
 * HyTech 2018 Vehicle Rear Control Unit
 * Monitor Shutdown Circuit initialization.
 * Control power to motor controller, fans, and pump.
 * Send wireless telemetry data via XBee.
 * Configured for Power Board rev4
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
Metro timer_imd_print_fault = Metro(500);
Metro timer_status_send = Metro(100);
Metro timer_fcu_restart_inverter = Metro(500); // Upon restart of the FCU, power cycle the inverter

/*
 * Global variables
 */
RCU_status rcu_status;

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
        if (msg.id == ID_FCU_RESTART) { // Restart inverter when the FCU restarts
            if (millis() > 1000) { // Ignore restart messages when this microcontroller has also just booted up
                inverter_restart = true;
                digitalWrite(SSR_INVERTER, LOW);
                timer_fcu_restart_inverter.reset();
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
     * Send status over CAN and XBee
     */
    if (timer_status_send.check()) {
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
     * Finish restarting the inverter when timer expires
     */
    if (inverter_restart && timer_fcu_restart_inverter.check()) {
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
