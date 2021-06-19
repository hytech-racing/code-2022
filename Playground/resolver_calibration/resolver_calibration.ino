/*
 * This program can be used to calibrate the motor Resolver. It sends a small torque
 * command to the motor once entered the Ready-to-Drive State. After 10 seconds,
 * the car goes into Tractive-System-Active and the motor controller stops sending
 * the torque command.
 *
 * Created March 2019 by Leonid Pozdneev.
 */


#include <ADC_SPI.h>
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Metro.h>
#include <Wire.h>
#include <MCP23S17.h>

/*
 * Teensy Pin definitions
 */
#define SSR_INVERTER 6
#define SSR_BRAKE_LIGHT 7
#define ADC_CS 9
#define EXPANDER_CS 10
#define SENSE_SHUTDOWN_B A2
#define SENSE_SHUTDOWN_C A3
#define SENSE_SHUTDOWN_E A0
#define SENSE_SHUTDOWN_F A1
#define PUMP_CTRL A6
#define SOFTWARE_SHUTDOWN_RELAY A7
#define FAN_1 A8
#define FAN_2 A9

/*
 * ADC pin definitions
 */
#define ADC_BRAKE_CHANNEL 0
#define ADC_ACCEL_1_CHANNEL 1
#define ADC_ACCEL_2_CHANNEL 2
#define ADC_12V_SUPPLY_CHANNEL 3
#define ADC_SHUTDOWN_D_READ_CHANNEL 4
#define ADC_BMS_OK_CHANNEL 5
#define ADC_OKHS_CHANNEL 6
#define ADC_TEMPSENSE_CHANNEL 7

/*
 * Expander pin definitions
 */
#define EXPANDER_BTN_RESTART_INVERTER 0
#define EXPANDER_BTN_START 1
#define EXPANDER_BTN_MODE 15
#define EXPANDER_READY_SOUND 8
#define EXPANDER_LED_START 9
#define EXPANDER_LED_BMS 10
#define EXPANDER_LED_IMD 11
#define EXPANDER_LED_POWER 12
#define EXPANDER_LED_MODE 13

/*
 * Global variables
 */
MCU_status mcu_status;
MCU_pedal_readings mcu_pedal_readings;
BMS_status bms_status;
BMS_temperatures bms_temperatures;

/*
 * Constant definitions
 */
 // TODO some of these values need to be calibrated once hardware is installed
#define BRAKE_ACTIVE 600
#define MIN_ACCELERATOR_PEDAL_1 100 // Low accelerator implausibility threshold
#define START_ACCELERATOR_PEDAL_1 250 // Position to start acceleration
#define END_ACCELERATOR_PEDAL_1 550 // Position to max out acceleration
#define MAX_ACCELERATOR_PEDAL_1 700 // High accelerator implausibility threshold
#define MIN_ACCELERATOR_PEDAL_2 3990 // Low accelerator implausibility threshold
#define START_ACCELERATOR_PEDAL_2 3840 // Position to start acceleration
#define END_ACCELERATOR_PEDAL_2 3550 // Position to max out acceleration
#define MAX_ACCELERATOR_PEDAL_2 3400 // High accelerator implausibility threshold
#define MIN_BRAKE_PEDAL 1510
#define MAX_BRAKE_PEDAL 1684
#define MIN_HV_VOLTAGE 500 // Volts in V * 0.1 - Used to check if Accumulator is energized
#define BMS_HIGH 134 // ~3V on BMS_OK line
#define IMD_HIGH 134 // ~3V on OKHS line
#define SHUTDOWN_B_HIGH 350 // ~5V on SHUTDOWN_B line
#define SHUTDOWN_C_HIGH 350 // ~5V on SHUTDOWN_C line
#define SHUTDOWN_D_HIGH 350 // ~5V on SHUTDOWN_D line ??????
#define SHUTDOWN_E_HIGH 350 // ~5V on SHUTDOWN_E line
#define SHUTDOWN_F_HIGH 350 // ~5V on SHUTDOWN_F line
#define FAN_1_DUTY_CYCLE 127 // TODO: figure out correct duty cycle (0 = 0%, 255 = 100%)
#define FAN_2_DUTY_CYCLE 127 // TODO: figure out correct duty cycle (0 = 0%, 255 = 100%)
#define BMS_HIGH_BATTERY_TEMPERATURE 50 // TODO: figure out correct value
#define GLV_VOLTAGE_MULTIPLIER 5.5963 // TODO:


/*
 * Timers  // old fcu timers
 */
Metro timer_accelerometer = Metro(100);
Metro timer_bms_imd_print_fault = Metro(500);
Metro timer_btn_restart_inverter = Metro(10);
Metro timer_btn_mode = Metro(50);
Metro timer_btn_start = Metro(50);
Metro timer_debug = Metro(200);
Metro timer_debug_raw_torque = Metro(200);
Metro timer_debug_torque = Metro(200);
Metro timer_ramp_torque = Metro(100);
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable
Metro timer_led_mode_blink_fast = Metro(150, 1);
Metro timer_led_mode_blink_slow = Metro(400, 1);
Metro timer_led_start_blink_fast = Metro(150);
Metro timer_led_start_blink_slow = Metro(400);
Metro timer_motor_controller_send = Metro(50);
Metro timer_ready_sound = Metro(2000); // Time to play RTD sound
Metro timer_can_update = Metro(100);

/*
 * Timers    // old rcu timers
 */ // TODO: figure out which ones we need/adjust the values
Metro timer_bms_print_fault = Metro(500);
Metro timer_debug_bms_status = Metro(1000);
Metro timer_debug_bms_temperatures = Metro(3000);
Metro timer_debug_bms_detailed_temperatures = Metro(3000);
Metro timer_debug_bms_voltages = Metro(1000);
Metro timer_debug_bms_detailed_voltages = Metro(3000);
Metro timer_debug_rms_command_message = Metro(200);
Metro timer_debug_rms_current_information = Metro(100);
Metro timer_debug_rms_fault_codes = Metro(2000);
Metro timer_debug_rms_internal_states = Metro(2000);
Metro timer_debug_rms_motor_position_information = Metro(100);
Metro timer_debug_rms_temperatures_1 = Metro(3000);
Metro timer_debug_rms_temperatures_3 = Metro(3000);
Metro timer_debug_rms_torque_timer_information = Metro(200);
Metro timer_debug_rms_voltage_information = Metro(100);
Metro timer_debug_fcu_status = Metro(2000);
Metro timer_debug_fcu_readings = Metro(200);
Metro timer_detailed_voltages = Metro(1000);
Metro timer_imd_print_fault = Metro(500);
Metro timer_restart_inverter = Metro(500, 1); // Allow the FCU to restart the inverter
Metro timer_status_send = Metro(100);

Metro timer_start_btn_read = Metro(50);

Metro timer_motor_controller_off = Metro(10000, 1);

bool btn_start_reading = true;
bool btn_mode_reading = true;
bool btn_restart_inverter_reading = true;


bool bms_imd_latched = false; // instead of the old rcu_status.set_bms_imd_latched(bool);
bool bms_faulting = false;
bool imd_faulting = false;
bool inverter_restart = false; // True when restarting the inverter

bool btn_start_debounced = false;
bool btn_start_current_state = false;

bool btn_start_debouncing = false;
uint8_t btn_start_new = 0;
bool btn_start_pressed = false;
bool btn_mode_debouncing = false;
uint8_t btn_mode_new = 0;
bool btn_mode_pressed = true;
bool btn_restart_inverter_debouncing = false;
bool btn_restart_inverter_pressed = false;
bool debug = true;
bool led_mode_active = false;
bool led_start_active = false;
bool regen_active = false;
uint8_t torque_mode = 0;
uint8_t led_mode_type = 0;
uint8_t led_start_type = 0; // 0 for off, 1 for steady, 2 for fast blink, 3 for slow blink
float rampRatio = 1;

uint16_t MAX_TORQUE = 1600; // Torque in Nm * 10
int16_t MAX_REGEN_TORQUE = 0;

static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
ADC_SPI ADC(ADC_CS);
MCP23S17 EXPANDER(0, EXPANDER_CS);
FlexCAN CAN(500000);

void setup() {
    EXPANDER.begin();

    EXPANDER.pinMode(EXPANDER_BTN_RESTART_INVERTER, INPUT);
    EXPANDER.pullupMode(EXPANDER_BTN_RESTART_INVERTER, HIGH);
    EXPANDER.pinMode(EXPANDER_BTN_MODE, INPUT);
    EXPANDER.pullupMode(EXPANDER_BTN_MODE, HIGH);
    EXPANDER.pinMode(EXPANDER_BTN_START, INPUT);
    EXPANDER.pullupMode(EXPANDER_BTN_START, HIGH);
    EXPANDER.pinMode(EXPANDER_LED_BMS, OUTPUT);
    EXPANDER.pinMode(EXPANDER_LED_IMD, OUTPUT);
    EXPANDER.pinMode(EXPANDER_LED_MODE, OUTPUT);
    EXPANDER.pinMode(EXPANDER_LED_POWER, OUTPUT);
    EXPANDER.pinMode(EXPANDER_LED_START, OUTPUT);
    EXPANDER.pinMode(EXPANDER_READY_SOUND, OUTPUT);
    pinMode(SOFTWARE_SHUTDOWN_RELAY, OUTPUT);
    pinMode(SSR_INVERTER, OUTPUT);
    pinMode(SSR_BRAKE_LIGHT, OUTPUT);
    pinMode(PUMP_CTRL, OUTPUT);
    pinMode(FAN_1, OUTPUT);
    pinMode(FAN_2, OUTPUT);
    pinMode(SENSE_SHUTDOWN_B, INPUT);
    pinMode(SENSE_SHUTDOWN_C, INPUT);
    pinMode(SENSE_SHUTDOWN_E, INPUT);

    Serial.begin(115200);
    CAN.begin();

    /* Configure CAN rx interrupt */
    interrupts();
    NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);
    attachInterruptVector(IRQ_CAN_MESSAGE,parse_can_message);
    FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;
    /* Configure CAN rx interrupt */

    delay(100);

    Serial.println("CAN system and serial communication initialized");

    //reset_inverter(); // what to do with this?? do we need to restart the inverter???
    set_state(MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
    digitalWrite(SOFTWARE_SHUTDOWN_RELAY, HIGH);
    digitalWrite(SSR_INVERTER, HIGH);
    digitalWrite(PUMP_CTRL, HIGH);
    //analogWrite(FAN_1, FAN_1_DUTY_CYCLE);
    //analogWrite(FAN_2, FAN_2_DUTY_CYCLE);
    mcu_status.set_bms_ok_high(true);
    mcu_status.set_imd_okhs_high(true);
    mcu_status.set_inverter_powered(true);
    digitalWrite(SSR_BRAKE_LIGHT, LOW);
}

void loop() {

    read_values();

    /*
     * State machine
     */
    switch (mcu_status.get_state()) {
        case MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
        break;

        case MCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
        // if start button has been pressed and brake pedal is held down, transition to the next state
        if (btn_start_pressed) {
            if (mcu_pedal_readings.get_brake_pedal_active()) {
                set_state(MCU_STATE_ENABLING_INVERTER);
                //Serial.println("ENABLING INVERTER");
            }
        }
        break;

        case MCU_STATE_ENABLING_INVERTER:
        if (timer_inverter_enable.check()) {
            set_state(MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        }
        break;

        case MCU_STATE_WAITING_READY_TO_DRIVE_SOUND:

        EXPANDER.digitalWrite(EXPANDER_READY_SOUND, HIGH);

        if (timer_ready_sound.check()) {
            set_state(MCU_STATE_READY_TO_DRIVE);
        }
        break;

        case MCU_STATE_READY_TO_DRIVE:
           // Serial.println("READY TO DRIVE");
        if (timer_motor_controller_send.check()) {
            MC_command_message mc_command_message = MC_command_message(50, 0, 0, 1, 0, 0);

            mc_command_message.write(tx_msg.buf);
            tx_msg.id = ID_MC_COMMAND_MESSAGE;
            tx_msg.len = 8;
            CAN.write(tx_msg);

        }
        if (timer_motor_controller_off.check()) {
            MC_command_message mc_command_message = MC_command_message(0, 0, 0, 0, 0, 0);

            mc_command_message.write(tx_msg.buf);
            tx_msg.id = ID_MC_COMMAND_MESSAGE;
            tx_msg.len = 8;
            CAN.write(tx_msg);

            set_state(MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        }
        break;

    }


    /*
     * Send a message to the Motor Controller over CAN when vehicle is not ready to drive
     */
    if (mcu_status.get_state() < MCU_STATE_READY_TO_DRIVE && timer_motor_controller_send.check()) {
        MC_command_message mc_command_message = MC_command_message(0, 0, 0, 0, 0, 0);
        if (mcu_status.get_state() >= MCU_STATE_ENABLING_INVERTER) {
             mc_command_message.set_inverter_enable(true);
        }
        mc_command_message.write(tx_msg.buf);
        tx_msg.id = ID_MC_COMMAND_MESSAGE;
        tx_msg.len = 8;//
        CAN.write(tx_msg);
    }


    read_dashboard_buttons();

    set_dashboard_leds();
}

/*
 * Parse incoming CAN messages
 */
void parse_can_message() {
    while (CAN.read(rx_msg)) {
        if (rx_msg.id == ID_MC_VOLTAGE_INFORMATION) {
            MC_voltage_information mc_voltage_information = MC_voltage_information(rx_msg.buf);
            if (mc_voltage_information.get_dc_bus_voltage() >= MIN_HV_VOLTAGE && mcu_status.get_state() == MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
                set_state(MCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
            }
            if (mc_voltage_information.get_dc_bus_voltage() < MIN_HV_VOLTAGE && mcu_status.get_state() > MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
                set_state(MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
            }
        }

        if (rx_msg.id == ID_MC_INTERNAL_STATES) {
            MC_internal_states mc_internal_states = MC_internal_states(rx_msg.buf);
            if (mc_internal_states.get_inverter_enable_state() && mcu_status.get_state() == MCU_STATE_ENABLING_INVERTER) {
                set_state(MCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
            }
        }

        if (rx_msg.id == ID_BMS_STATUS) {
            bms_status.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_TEMPERATURES) {
            bms_temperatures.load(rx_msg.buf);
        }
    }

}

void reset_inverter() {
    inverter_restart = true;
    digitalWrite(SSR_INVERTER, LOW);
    timer_restart_inverter.reset();
    mcu_status.set_inverter_powered(false);
}

/*
 * Read values of sensors
 */
void read_values() {

    noInterrupts();
    mcu_pedal_readings.set_accelerator_pedal_raw_1(ADC.read_adc(ADC_ACCEL_1_CHANNEL));
    mcu_pedal_readings.set_accelerator_pedal_raw_2(ADC.read_adc(ADC_ACCEL_2_CHANNEL));
    mcu_pedal_readings.set_brake_pedal_raw(ADC.read_adc(ADC_BRAKE_CHANNEL));
    interrupts();

    if (mcu_pedal_readings.get_brake_pedal_raw() >= BRAKE_ACTIVE) {
        mcu_pedal_readings.set_brake_pedal_active(true);
    } else {
        mcu_pedal_readings.set_brake_pedal_active(false);
    }
    if (debug && timer_debug.check()) {
        Serial.print("MCU PEDAL ACCEL 1: ");
        //Serial.println(accel1_reading);
        Serial.println(mcu_pedal_readings.get_accelerator_pedal_raw_1());
        Serial.print("MCU PEDAL ACCEL 2: ");
        //Serial.println(accel2_reading);
        Serial.println(mcu_pedal_readings.get_accelerator_pedal_raw_2());
        Serial.print("MCU PEDAL BRAKE: ");
        //Serial.println(brake_reading);
        Serial.println(mcu_pedal_readings.get_brake_pedal_raw());
        Serial.print("MCU BRAKE ACT: ");
        Serial.println(mcu_pedal_readings.get_brake_pedal_active());
        Serial.print("MCU STATE: ");
        Serial.println(mcu_status.get_state());
    }
    // TODO calculate temperature
}

/*
 * Set the Mode LED
 */
void set_mode_led(uint8_t type) {
    if (led_mode_type != type) {
        led_mode_type = type;

        if (type == 0) {
            if (debug) {
                Serial.println("MCU Setting Mode LED off");
            }
            return;
        }
        if (type == 1) {
            if (debug) {
                Serial.println("MCU Setting Mode LED solid on");
            }
        } else if (type == 2) {
            timer_led_mode_blink_fast.reset();
            if (debug) {
                Serial.println("MCU Setting Mode LED fast blink");
            }
        } else if (type == 3) {
            timer_led_mode_blink_slow.reset();
            if (debug) {
                Serial.println("MCU Setting Mode LED slow blink");
            }
        }
    }
}

/*
 * Set the Start LED
 */
void set_start_led(uint8_t type) {
    if (led_start_type != type) {
        led_start_type = type;

        if (type == 0) {
            if (debug) {
                Serial.println("MCU Setting Start LED off");
            }
            return;
        }
        if (type == 1) {
            if (debug) {
                Serial.println("MCU Setting Start LED solid on");
            }
        } else if (type == 2) {
            timer_led_start_blink_fast.reset();
            if (debug) {
                Serial.println("MCU Setting Start LED fast blink");
            }
        } else if (type == 3) {
            timer_led_start_blink_slow.reset();
            if (debug) {
                Serial.println("MCU Setting Start LED slow blink");
            }
        }
    }
}

/*
 * Handle changes in state
 */
void set_state(uint8_t new_state) {
    if (mcu_status.get_state() == new_state) {
        return;
    }
    mcu_status.set_state(new_state);
    if (new_state == MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
        set_start_led(0);
        //Serial.println(new_state);
    }
    if (new_state == MCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
        set_start_led(2);
        //Serial.println(new_state);
    }
    if (new_state == MCU_STATE_ENABLING_INVERTER) {
        set_start_led(1);
        Serial.println("MCU Enabling inverter");
        MC_command_message mc_command_message = MC_command_message(0, 0, 0, 1, 0, 0);
        tx_msg.id = 0xC0;
        tx_msg.len = 8;
        for(int i = 0; i < 10; i++) {
            mc_command_message.write(tx_msg.buf); // many enable commands
            CAN.write(tx_msg);
        }
        mc_command_message.set_inverter_enable(false);
        mc_command_message.write(tx_msg.buf); // disable command
        CAN.write(tx_msg);
        for(int i = 0; i < 10; i++) {
            mc_command_message.set_inverter_enable(true);
            mc_command_message.write(tx_msg.buf); // many more enable commands
            CAN.write(tx_msg);
        }
        Serial.println("MCU Sent enable command");
        timer_inverter_enable.reset();
    }
    if (new_state == MCU_STATE_WAITING_READY_TO_DRIVE_SOUND) {
        timer_ready_sound.reset();
        //EXPANDER.digitalWrite(EXPANDER_READY_SOUND, HIGH);
        Serial.println("Inverter enabled");
        Serial.println("RTDS enabled");
    }
    if (new_state == MCU_STATE_READY_TO_DRIVE) {
        EXPANDER.digitalWrite(EXPANDER_READY_SOUND, LOW);
        Serial.println("RTDS deactivated");
        Serial.println("Ready to drive");
        timer_motor_controller_off.reset();
    }
}

// adjust for different torque maps
int calculate_torque() {
    int calculated_torque = 0;

    if (!mcu_pedal_readings.get_accelerator_implausibility()) {
        int torque1 = map(mcu_pedal_readings.get_accelerator_pedal_raw_1(), START_ACCELERATOR_PEDAL_1, END_ACCELERATOR_PEDAL_1, 0, MAX_TORQUE);
        int torque2 = map(mcu_pedal_readings.get_accelerator_pedal_raw_2(), START_ACCELERATOR_PEDAL_2, END_ACCELERATOR_PEDAL_2, 0, MAX_TORQUE);

        // torque values are greater than the max possible value, set them to max
        if (torque1 > MAX_TORQUE) {
            torque1 = MAX_TORQUE;
        }
        if (torque2 > MAX_TORQUE) {
            torque2 = MAX_TORQUE;
        }
        // compare torques to check for accelerator implausibility
        if (abs(torque1 - torque2) * 100 / MAX_TORQUE > 10) {
            mcu_pedal_readings.set_accelerator_implausibility(true);
            Serial.print("ACCEL IMPLAUSIBILITY: COMPARISON FAILED");
        } else {
            calculated_torque = min(torque1, torque2); // add ramp?

            if (debug && timer_debug_raw_torque.check()) {
                Serial.print("TORQUE REQUEST DELTA PERCENT: "); // Print the % difference between the 2 accelerator sensor requests
                Serial.println(abs(torque1 - torque2) / (double) MAX_TORQUE * 100);
                Serial.print("MCU RAW TORQUE: ");
                Serial.println(calculated_torque);
            }
            if (calculated_torque > MAX_TORQUE) {
                calculated_torque = MAX_TORQUE;
            }
            if (calculated_torque < 0) {
                calculated_torque = 0;
            }
            // if regen is active and pedal is not pressed, send negative torque for regen
            // TODO: improve this
            if (regen_active && calculated_torque == 0) {
                calculated_torque = MAX_REGEN_TORQUE;
            }
        }
    }

    return calculated_torque;
}

void read_dashboard_buttons() {
    int dash_reading = EXPANDER.digitalRead();
    //Serial.println(dash_reading, BIN);
    btn_start_reading = (dash_reading >> EXPANDER_BTN_START) & 0x1;
    btn_mode_reading = (dash_reading >> EXPANDER_BTN_MODE) & 0x1;
    btn_restart_inverter_reading = (dash_reading >> EXPANDER_BTN_RESTART_INVERTER) & 0x1;

    // debounce start button
    if (btn_start_reading == btn_start_pressed && !btn_start_debouncing) { // Value is different than stored
        btn_start_debouncing = true;
        timer_btn_start.reset();
    }
    if (btn_start_debouncing && btn_start_reading != btn_start_pressed) { // Value returns during debounce period
        btn_start_debouncing = false;
    }
    if (btn_start_debouncing && timer_btn_start.check()) { // Debounce period finishes without value returning
        btn_start_pressed = !btn_start_pressed;
    }

    // debounce torque mode button
    if (btn_mode_reading == btn_mode_pressed && !btn_mode_debouncing) {    // value different than stored
        btn_mode_debouncing = true;
        timer_btn_mode.reset();
    }
    if (btn_mode_debouncing && btn_mode_reading != btn_mode_pressed) {     // value returns during debounce period
        btn_mode_debouncing = false;
    }
    if (btn_mode_debouncing && timer_btn_mode.check()) {                        // debounce period finishes
        btn_mode_pressed = !btn_mode_pressed;
        if (btn_mode_pressed) {
            torque_mode = (torque_mode + 1) % 4;
            if (torque_mode == 0) {
                set_mode_led(0);
            } else if (torque_mode == 1) {
                set_mode_led(1);
            } else if (torque_mode == 2) {
                set_mode_led(2);
            } else if (torque_mode == 3) {
                set_mode_led(3);
            }
        }
    }

    // debounce restart inverter button
    if (btn_restart_inverter_reading == btn_restart_inverter_pressed && !btn_restart_inverter_debouncing) { // value different than stored
        btn_restart_inverter_debouncing = true;
        timer_btn_restart_inverter.reset();
    }
    if (btn_restart_inverter_debouncing && btn_restart_inverter_reading != btn_restart_inverter_pressed) {  // value returns during debounce period
        btn_restart_inverter_debouncing = false;
    }
    if (btn_restart_inverter_debouncing && timer_btn_restart_inverter.check()) {
        btn_restart_inverter_pressed = !btn_restart_inverter_pressed;
        //reset_inverter();
    }
}

void set_dashboard_leds() {
    if ((led_mode_type == 2 && timer_led_mode_blink_fast.check()) || (led_mode_type == 3 && timer_led_mode_blink_slow.check())) {
        EXPANDER.digitalWrite(EXPANDER_LED_MODE, led_mode_active);
        led_mode_active = !led_mode_active;
    }
    if (led_mode_type < 2) {
        led_mode_active = led_mode_type;
        EXPANDER.digitalWrite(EXPANDER_LED_MODE, led_mode_active);
    }

    /*
     * Blink start led
     */
    if ((led_start_type == 2 && timer_led_start_blink_fast.check()) || (led_start_type == 3 && timer_led_start_blink_slow.check())) {
        EXPANDER.digitalWrite(EXPANDER_LED_START, led_start_active);
        led_start_active = !led_start_active;
    }
    if (led_start_type < 2) {
        led_start_active = led_start_type;
        EXPANDER.digitalWrite(EXPANDER_LED_START, led_start_active);
    }
}