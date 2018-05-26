/*
 * HyTech 2018 Vehicle Front Control Unit
 * Interface with dashboard lights, buttons, and buzzer.
 * Read pedal sensor values and communicate with motor controller.
 * Configured for Front Control Unit rev6
 */
#include "ADC_SPI.h"
#include <FlexCAN.h>
#include "HyTech17.h"
#include <Metro.h>

/*
 * Pin definitions
 */
#define ADC_ACCEL_1_CHANNEL 0
#define ADC_ACCEL_2_CHANNEL 1
#define ADC_BRAKE_CHANNEL 2
#define ADC_SPI_CS 10
#define BTN_CYCLE A3
#define BTN_MODE A1
#define BTN_START A6
#define LED_BMS 6
#define LED_IMD 7
#define LED_MODE 9
#define LED_POWER 8
#define LED_START 5
#define READY_SOUND 2
#define SOFTWARE_SHUTDOWN_RELAY A2

/*
 * Constant definitions
 */
// TODO some of these values need to be calibrated once hardware is installed
#define BRAKE_ACTIVE 600
#define MIN_ACCELERATOR_PEDAL_1 250 // compare pedal travel
#define MAX_ACCELERATOR_PEDAL_1 550
#define MIN_ACCELERATOR_PEDAL_2 3840
#define MAX_ACCELERATOR_PEDAL_2 3550
#define MIN_BRAKE_PEDAL 1510
#define MAX_BRAKE_PEDAL 1684
#define MIN_HV_VOLTAGE 500 // Volts in V * 0.1 - Used to check if Accumulator is energized

/*
 * Timers
 */
Metro timer_bms_imd_print_fault = Metro(500);
Metro timer_btn_cycle = Metro(10);
Metro timer_btn_mode = Metro(10);
Metro timer_btn_start = Metro(10);
Metro timer_debug = Metro(200);
Metro timer_debug_raw_torque = Metro(200);
Metro timer_debug_torque = Metro(200);
Metro timer_ramp_torque = Metro(100);
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable
Metro timer_led_start_blink_fast = Metro(150);
Metro timer_led_start_blink_slow = Metro(400);
Metro timer_motor_controller_send = Metro(50);
Metro timer_ready_sound = Metro(2000); // Time to play RTD sound
Metro timer_can_update = Metro(100);

/*
 * Global variables
 */
BMS_status bms_status;
FCU_status fcu_status;
FCU_readings fcu_readings;
RCU_status rcu_status;

bool btn_start_debouncing = false;
uint8_t btn_start_new = 0;
bool btn_start_pressed = false;
bool btn_mode_debouncing = false;
uint8_t btn_mode_new = 0;
bool btn_mode_pressed = true;
bool btn_cycle_debouncing = false;
bool btn_cycle_pressed = false;
bool debug = true;
bool led_start_active = false;
bool regen_active = false;
bool low_torque = false;
uint8_t led_start_type = 0; // 0 for off, 1 for steady, 2 for fast blink, 3 for slow blink
float rampRatio = 1;

int MAX_TORQUE = 1600; // Torque in Nm * 10

ADC_SPI ADC(ADC_SPI_CS);
FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
    pinMode(BTN_CYCLE, INPUT_PULLUP);
    pinMode(BTN_MODE, INPUT_PULLUP);
    pinMode(BTN_START, INPUT_PULLUP);
    pinMode(LED_BMS, OUTPUT);
    pinMode(LED_IMD, OUTPUT);
    pinMode(LED_MODE, OUTPUT);
    pinMode(LED_POWER, OUTPUT);
    pinMode(LED_START, OUTPUT);
    pinMode(READY_SOUND, OUTPUT);
    pinMode(SOFTWARE_SHUTDOWN_RELAY, OUTPUT);

    Serial.begin(115200); // Init serial for PC communication
    CAN.begin(); // Init CAN for vehicle communication
    delay(100);
    Serial.println("CAN system and serial communication initialized");

    set_state(FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
    reset_inverter();
}

void loop() {
    while (CAN.read(msg)) {
        if (msg.id == ID_RCU_STATUS) {
            rcu_status.load(msg.buf);
        }

        if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
            MC_voltage_information mc_voltage_information = MC_voltage_information(msg.buf);
            if (mc_voltage_information.get_dc_bus_voltage() >= MIN_HV_VOLTAGE && fcu_status.get_state() == FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
                set_state(FCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
            }
            if (mc_voltage_information.get_dc_bus_voltage() < MIN_HV_VOLTAGE && fcu_status.get_state() > FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
                set_state(FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
            }
        }

        if (msg.id == ID_MC_INTERNAL_STATES) {
            MC_internal_states mc_internal_states = MC_internal_states(msg.buf);
            if (mc_internal_states.get_inverter_enable_state() && fcu_status.get_state() == FCU_STATE_ENABLING_INVERTER) {
                set_state(FCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
            }
        }

        if (msg.id == ID_BMS_STATUS) {
            bms_status.load(msg.buf);
        }
    }

    /*
     * Send state over CAN
     */
    if (timer_can_update.check()) {
        // Send Front Control Unit message
        fcu_status.set_accelerator_boost_mode(0);
        fcu_status.write(msg.buf);
        msg.id = ID_FCU_STATUS;
        msg.len = sizeof(CAN_message_fcu_status_t);
        CAN.write(msg);

        // Send second Front Control Unit message
        read_values(); // Calculate new values to send
        fcu_readings.write(msg.buf);
        msg.id = ID_FCU_READINGS;
        msg.len = sizeof(CAN_message_fcu_readings_t);
        CAN.write(msg);
    }

    /*
     * State machine
     */
    switch (fcu_status.get_state()) {
        case FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
        break;

        case FCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
        if (btn_start_new == fcu_status.get_start_button_press_id()) { // Start button has been pressed
            if (fcu_status.get_brake_pedal_active()) { // Required to hold brake pedal to activate motor controller
                set_state(FCU_STATE_ENABLING_INVERTER);
            } else {
                btn_start_new = fcu_status.get_start_button_press_id() + 1;
            }
        }
        break;

        case FCU_STATE_ENABLING_INVERTER:
        if (timer_inverter_enable.check()) { // Inverter enable timeout
            set_state(FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        }
        break;

        case FCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
        if (timer_ready_sound.check()) { // RTDS has sounded
            set_state(FCU_STATE_READY_TO_DRIVE);
        }
        break;

        case FCU_STATE_READY_TO_DRIVE:
        if (timer_motor_controller_send.check()) {
            MC_command_message mc_command_message = MC_command_message(0, 0, 0, 1, 0, 0);
            read_values(); // Read new sensor values

            // Check for accelerator implausibility FSAE EV2.3.10
            fcu_status.set_accelerator_implausibility(false);
            if (fcu_readings.get_accelerator_pedal_raw_1() < MIN_ACCELERATOR_PEDAL_1 || fcu_readings.get_accelerator_pedal_raw_1() > MAX_ACCELERATOR_PEDAL_1) {
                fcu_status.set_accelerator_implausibility(true);
            }
            if (fcu_readings.get_accelerator_pedal_raw_2() > MIN_ACCELERATOR_PEDAL_2 || fcu_readings.get_accelerator_pedal_raw_2() < MAX_ACCELERATOR_PEDAL_2) {
                fcu_status.set_accelerator_implausibility(true);
            }

            // Calculate torque value
            int calculated_torque = 0;
            if (!fcu_status.get_accelerator_implausibility()) {
                int torque1 = map(fcu_readings.get_accelerator_pedal_raw_1(), MIN_ACCELERATOR_PEDAL_1, MAX_ACCELERATOR_PEDAL_1, 0, MAX_TORQUE);
                int torque2 = map(fcu_readings.get_accelerator_pedal_raw_2(), MIN_ACCELERATOR_PEDAL_2, MAX_ACCELERATOR_PEDAL_2, 0, MAX_TORQUE);
                if (abs(torque1 - torque2) * 100 / MAX_TORQUE > 10) { // Second accelerator implausibility check FSAE EV2.3.6
                    fcu_status.set_accelerator_implausibility(true);
                    Serial.print("ACCEL IMPLAUSIBILITY: COMPARISON FAILED");
                } else {
                    calculated_torque = (int) (min(torque1, torque2) * rampRatio);

                    if (rampRatio < 1 && timer_ramp_torque.check()) {
                        rampRatio += 0.1;
                        if (rampRatio > 1) {
                            rampRatio = 1;
                        }
                    }
                    if (debug && timer_debug_raw_torque.check()) {
                        Serial.print("TORQUE REQUEST DELTA PERCENT: "); // Print the % difference between the 2 accelerator sensor requests
                        Serial.println(abs(torque1 - torque2) / (double) MAX_TORQUE * 100);
                        Serial.print("FCU RAW TORQUE: ");
                        Serial.println(calculated_torque);
                    }
                    if (calculated_torque > MAX_TORQUE) {
                        calculated_torque = MAX_TORQUE;
                    }
                    if (calculated_torque < 0) {
                        calculated_torque = 0;
                    }
                    // if regen is active and pedal is not pressed, send negative torque for regen
                    // if (regen_active && calculated_torque == 0) {
                    //     calculated_torque = -20;
                    // }
                }
            }

            // FSAE EV2.5 APPS / Brake Pedal Plausibility Check
            if (fcu_status.get_brake_implausibility() && !fcu_status.get_brake_pedal_active() && calculated_torque <= (MAX_TORQUE / 4)) {
                fcu_status.set_brake_implausibility(false); // Clear implausibility
            }
            if (fcu_status.get_brake_pedal_active() && calculated_torque > (MAX_TORQUE / 4)) {
                fcu_status.set_brake_implausibility(true);
            }

            if (fcu_status.get_brake_implausibility() || fcu_status.get_accelerator_implausibility()) {
                // Implausibility exists, command 0 torque
                calculated_torque = 0;
                rampRatio = 0;
            }

            // FSAE FMEA specifications
            if (!rcu_status.get_bms_ok_high()) {
                calculated_torque = 0;
            }
            
            if (!rcu_status.get_imd_okhs_high()) {
                calculated_torque = 0;
            }
            
            if (debug && timer_debug_torque.check()) {
                Serial.print("FCU REQUESTED TORQUE: ");
                Serial.println(calculated_torque);
                Serial.print("FCU IMPLAUS THROTTLE: ");
                Serial.println(fcu_status.get_accelerator_implausibility());
                Serial.print("FCU IMPLAUS BRAKE: ");
                Serial.println(fcu_status.get_brake_implausibility());
            }
            
            mc_command_message.set_torque_command(calculated_torque);

            mc_command_message.write(msg.buf);
            msg.id = ID_MC_COMMAND_MESSAGE;
            msg.len = 8;
            CAN.write(msg);
        }
        break;
    }

    /*
     * Send a message to the Motor Controller over CAN when vehicle is not ready to drive
     */
    if (fcu_status.get_state() < FCU_STATE_READY_TO_DRIVE && timer_motor_controller_send.check()) {
        MC_command_message mc_command_message = MC_command_message(0, 0, 0, 0, 0, 0);
        if (fcu_status.get_state() >= FCU_STATE_ENABLING_INVERTER) {
            mc_command_message.set_inverter_enable(true);
        }
        mc_command_message.write(msg.buf);
        msg.id = ID_MC_COMMAND_MESSAGE;
        msg.len = 8;
        CAN.write(msg);
    }

    /*
     * Blink start led
     */
    if ((led_start_type == 2 && timer_led_start_blink_fast.check()) || (led_start_type == 3 && timer_led_start_blink_slow.check())) {
        if (led_start_active) {
            digitalWrite(LED_START, LOW);
        } else {
            digitalWrite(LED_START, HIGH);
        }
        led_start_active = !led_start_active;
    }

    /*
     * Handle start button press and depress
     */
    if (digitalRead(BTN_START) == btn_start_pressed && !btn_start_debouncing) { // Value is different than stored
        btn_start_debouncing = true;
        timer_btn_start.reset();
    }
    if (btn_start_debouncing && digitalRead(BTN_START) != btn_start_pressed) { // Value returns during debounce period
        btn_start_debouncing = false;
    }
    if (btn_start_debouncing && timer_btn_start.check()) { // Debounce period finishes without value returning
        btn_start_pressed = !btn_start_pressed;
        if (btn_start_pressed) {
            fcu_status.set_start_button_press_id(fcu_status.get_start_button_press_id() + 1);
            if (debug) {
                Serial.print("FCU START BUTTON ID: ");
                Serial.println(fcu_status.get_start_button_press_id());
            }
        }
    }

    /*
     * Handle Regen button press and depress
     */
    if (digitalRead(BTN_MODE) == btn_mode_pressed && !btn_mode_debouncing) {    // value different than stored
        btn_mode_debouncing = true;
        timer_btn_mode.reset();
    }
    if (btn_mode_debouncing && digitalRead(BTN_MODE) != btn_mode_pressed) {     // value returns during debounce period
        btn_mode_debouncing = false;
    }
    if (btn_mode_debouncing && timer_btn_mode.check()) {                        // debounce period finishes
        btn_mode_pressed = !btn_mode_pressed;
        if (btn_mode_pressed) {
            low_torque = !low_torque;
        }
    }

    if (digitalRead(BTN_CYCLE) == btn_cycle_pressed && !btn_cycle_debouncing) { // value different than stored
        btn_cycle_debouncing = true;
        timer_btn_cycle.reset();
    }
    if (btn_cycle_debouncing && digitalRead(BTN_CYCLE) != btn_cycle_pressed) {  // value returns during debounce period
        btn_cycle_debouncing = false;
    }
    if (btn_cycle_debouncing && timer_btn_cycle.check()) {
        btn_cycle_pressed = !btn_cycle_pressed;
        reset_inverter();
    }

    /*
     * Handle torque mode toggling after button pressed
     */
    if (low_torque) {
        digitalWrite(LED_MODE, HIGH);
        MAX_TORQUE = 1500;
    } else {
        digitalWrite(LED_MODE, LOW);
        MAX_TORQUE = 1600;
    }

    /*
     * Open shutdown circuit if we detect a BMS or IMD fault; this is in addition to the latching relay hardware
     */
    digitalWrite(SOFTWARE_SHUTDOWN_RELAY, !(bms_status.get_error_flags() || !rcu_status.get_bms_ok_high() || !rcu_status.get_imd_okhs_high()));
    digitalWrite(LED_BMS, bms_status.get_error_flags() || !rcu_status.get_bms_ok_high());
    digitalWrite(LED_IMD, !rcu_status.get_imd_okhs_high());
    digitalWrite(LED_POWER, rcu_status.get_bms_imd_latched());
    if (debug && timer_bms_imd_print_fault.check()) {
        if (!rcu_status.get_bms_ok_high() || bms_status.get_error_flags()) {
            Serial.println("RCU BMS FAULT: detected");
        }
        if (!rcu_status.get_imd_okhs_high()) {
            Serial.println("RCU IMD FAULT: detected");
        }
    }
}

/*
 * Read values of sensors
 */
void read_values() {
    fcu_readings.set_accelerator_pedal_raw_1(ADC.read_adc(ADC_ACCEL_1_CHANNEL));
    fcu_readings.set_accelerator_pedal_raw_2(ADC.read_adc(ADC_ACCEL_2_CHANNEL));
    fcu_readings.set_brake_pedal_raw(ADC.read_adc(ADC_BRAKE_CHANNEL));
    if (fcu_readings.get_brake_pedal_raw() >= BRAKE_ACTIVE) {
        fcu_status.set_brake_pedal_active(true);
    } else {
        fcu_status.set_brake_pedal_active(false);
    }
    if (debug && timer_debug.check()) {
        Serial.print("FCU PEDAL THROTTLE 1: ");
        Serial.println(fcu_readings.get_accelerator_pedal_raw_1());
        Serial.print("FCU PEDAL THROTTLE 2: ");
        Serial.println(fcu_readings.get_accelerator_pedal_raw_2());
        Serial.print("FCU PEDAL BRAKE: ");
        Serial.println(fcu_readings.get_brake_pedal_raw());
        Serial.print("FCU BRAKE ACT: ");
        Serial.println(fcu_status.get_brake_pedal_active());
        Serial.print("FCU STATE: ");
        Serial.println(fcu_status.get_state());
    }
    // TODO calculate temperature
}

/*
 * Set the Start LED
 */
void set_start_led(uint8_t type) {
    if (led_start_type != type) {
        led_start_type = type;

        if (type == 0) {
            digitalWrite(LED_START, LOW);
            led_start_active = false;
            if (debug) {
                Serial.println("FCU Setting Start LED off");
            }
            return;
        }

        digitalWrite(LED_START, HIGH);
        led_start_active = true;

        if (type == 1) {
            if (debug) {
                Serial.println("FCU Setting Start LED solid on");
            }
        } else if (type == 2) {
            timer_led_start_blink_fast.reset();
            if (debug) {
                Serial.println("FCU Setting Start LED fast blink");
            }
        } else if (type == 3) {
            timer_led_start_blink_slow.reset();
            if (debug) {
                Serial.println("FCU Setting Start LED slow blink");
            }
        }
    }
}

/*
 * Send restart message, so RCU will power cycle the inverter
 * Used at FCU restart to clear the inverter's CAN message timeout fault
 * Also used manually by the driver to clear other motor controller faults
 */
void reset_inverter() {
    msg.id = ID_RCU_RESTART_MC;
    msg.len = 1;
    CAN.write(msg);
}

/*
 * Handle changes in state
 */
void set_state(uint8_t new_state) {
    if (fcu_status.get_state() == new_state) {
        return;
    }
    fcu_status.set_state(new_state);
    if (new_state == FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
        set_start_led(0);
    }
    if (new_state == FCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
        set_start_led(2);
        btn_start_new = fcu_status.get_start_button_press_id() + 1;
    }
    if (new_state == FCU_STATE_ENABLING_INVERTER) {
        set_start_led(1);
        Serial.println("FCU Enabling inverter");
        MC_command_message mc_command_message = MC_command_message(0, 0, 0, 1, 0, 0);
        msg.id = 0xC0;
        msg.len = 8;
        for(int i = 0; i < 10; i++) {
            mc_command_message.write(msg.buf); // many enable commands
            CAN.write(msg);
        }
        mc_command_message.set_inverter_enable(false);
        mc_command_message.write(msg.buf); // disable command
        CAN.write(msg);
        for(int i = 0; i < 10; i++) {
            mc_command_message.set_inverter_enable(true);
            mc_command_message.write(msg.buf); // many more enable commands
            CAN.write(msg);
        }
        Serial.println("FCU Sent enable command");
        timer_inverter_enable.reset();
    }
    if (new_state == FCU_STATE_WAITING_READY_TO_DRIVE_SOUND) {
        timer_ready_sound.reset();
        digitalWrite(READY_SOUND, HIGH);
        Serial.println("Inverter enabled");
        Serial.println("RTDS enabled");
    }
    if (new_state == FCU_STATE_READY_TO_DRIVE) {
        digitalWrite(READY_SOUND, LOW);
        Serial.println("RTDS deactivated");
        Serial.println("Ready to drive");
    }
}