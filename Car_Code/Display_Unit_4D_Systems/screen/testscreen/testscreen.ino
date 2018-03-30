/*
 * testscreen.ino - Test 4D Systems Gen4-ULCD-43D Code
 * Brian Cai
 * 3/27/2018
 */
#include <FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>

/*
 */
FlexCAN CAN(500000);
static CAN_message_t msg;
MC_current_information MCcurrentInfo;
int leddigits4 = 10;
//                    leddigits4 = MCcurrentInfo.get_phase_a_current();//Current Draw
BMS_temperatures BMStemps;
int leddigits0 = 11;
int leddigits1 = 12;
//                    leddigits0 = BMStemps.get_high_temperature(); //battery temp high
//                    leddigits1 = BMStemps.get_low_temperature(); //battery temp low
MC_motor_position_information motorposition;
int motorposition = 100;
//                    motorposition = MC_motor_position_information(msg.buf);
RCU_status rcu_status;
int leddigits3 = 13;
//                    leddigits3 = rcu_status.get_glv_battery_voltage()*100; //2 decimal points
MC_temperatures_3 mctemp3;
int leddigits2 = 14;
//                    leddigits2 = mctemp3.get_motor_temperature()*10; //1 decimal point
BMS_voltages bmsvoltages;
int totalvoltage = 15;
//                    int totalvoltage = bmsvoltages.get_total(); //total voltage

/*
 * Timers
 */
Metro timer_imd_print_fault = Metro(500);
Metro timer_status_send = Metro(500); // this is longer for debug purposes
Metro timer_fcu_restart_inverter = Metro(500); // Upon restart of the FCU, power cycle the inverter

Metro timer_debug_current_draw = Metro(1000);
Metro timer_debug_bms_temperature_high = Metro(1000);
Metro timer_debug_bms_temperature_low = Metro(1000);
Metro timer_debug_motor_rotation = Metro(1000);
Metro timer_debug_glv_voltage = Metro(2000);
Metro timer_debug_motor_temperature = Metro(2000);
Metro timer_debug_total_voltage = Metro(2000);

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

/*
 * Global variables
 */
RCU_status rcu_status;
boolean bms_faulting = false;
boolean imd_faulting = false;
boolean inverter_restart = false; // True when restarting the inverter
void setup() {
    pinMode(COOL_RELAY_1, OUTPUT);
    pinMode(COOL_RELAY_2, OUTPUT);
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
//        if ((msg.id == ID_MC_TEMPERATURES_1 && timer_debug_rms_temperatures_1.check())
//                || (msg.id == ID_MC_TEMPERATURES_3 && timer_debug_rms_temperatures_3.check())
//                || (msg.id == ID_MC_MOTOR_POSITION_INFORMATION && timer_debug_rms_motor_position_information.check())
//                || (msg.id == ID_MC_CURRENT_INFORMATION && timer_debug_rms_current_information.check())
//                || (msg.id == ID_MC_VOLTAGE_INFORMATION && timer_debug_rms_voltage_information.check())
//                || (msg.id == ID_MC_INTERNAL_STATES && timer_debug_rms_internal_states.check())
//                || (msg.id == ID_MC_FAULT_CODES && timer_debug_rms_fault_codes.check())
//                || (msg.id == ID_MC_TORQUE_TIMER_INFORMATION && timer_debug_rms_torque_timer_information.check())
//                || (msg.id == ID_BMS_VOLTAGES && timer_debug_bms_voltages.check())
//                || (msg.id == ID_BMS_TEMPERATURES && timer_debug_bms_temperatures.check())
//                || (msg.id == ID_BMS_STATUS && timer_debug_bms_status.check())
//                || (msg.id == ID_FCU_STATUS && timer_debug_fcu_status.check())) {
//            Serial.println("Sent non-status CAN Message");
//            write_xbee_data();
//        }
    }
    // ***** TESTING PURPOSES ONLY -- NOT REAL DATA
    if (timer_debug_bms_voltages.check()) {
        msg.id = ID_BMS_VOLTAGES;
        msg.len = sizeof(CAN_message_bms_voltages_t);
        BMS_voltages voltage_info;
        voltage_info.set_average(30);
        voltage_info.set_low(28);
        voltage_info.set_high(33);
        voltage_info.set_total(3000);
        voltage_info.write(msg.buf);
        Serial.println("Sent BMS Voltages");
        write_xbee_data();
    }
    if (timer_debug_fcu_status.check()) {
        msg.id = ID_FCU_STATUS;
        msg.len = sizeof(CAN_message_fcu_status_t);
        FCU_status fcu_status;
        fcu_status.set_state(FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        fcu_status.set_accelerator_implausibility(0);
        fcu_status.set_accelerator_boost_mode(0);
        fcu_status.set_brake_implausibility(0);
        fcu_status.set_brake_pedal_active(1);
        fcu_status.set_start_button_press_id(3);
        fcu_status.write(msg.buf);
        Serial.println("Sent FCU Status");
        write_xbee_data();
    }
    // ***** END TESTING CODE
    /*
     * Send status over CAN and XBee
     */
    if (timer_status_send.check()) {
        // ***** TESTING PURPOSES ONLY -- NOT REAL VOLTAGE OR TEMPERATURE
        rcu_status.set_glv_battery_voltage(127);
        rcu_status.set_temperature(2055);
        // ***** END TESTING CODE
        rcu_status.write(msg.buf);
        msg.id = ID_RCU_STATUS;
        msg.len = sizeof(CAN_message_rcu_status_t);
//        CAN.write(msg);
//        Serial.print("STATUS MESSAGE ");
//        Serial.print(msg.id, HEX);
//        Serial.print(": ");
//        for (int i = 0; i < msg.len; i++) {
//            Serial.print(msg.buf[i]);
//            Serial.print(" ");
//        }
//        Serial.println();
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
    } else {
        rcu_status.set_bms_imd_latched(false);
        digitalWrite(COOL_RELAY_2, LOW);
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
    uint8_t xb_buf[XBEE_PKT_LEN] = {0};
    memcpy(xb_buf, &msg.id, sizeof(msg.id));        // msg id
    memcpy(xb_buf + sizeof(msg.id), &msg.len, sizeof(msg.len));     // msg len
    memcpy(xb_buf + sizeof(msg.id) + sizeof(msg.len), msg.buf, msg.len); // msg contents
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


