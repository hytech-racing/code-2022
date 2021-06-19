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
#define EXPANDER_LED_MC_ERR 12
#define EXPANDER_LED_MODE 13

/*
 * Global variables
 */
MCU_status mcu_status;
MCU_pedal_readings mcu_pedal_readings;
BMS_status bms_status;
BMS_temperatures bms_temperatures;
BMS_voltages bms_voltages;
MC_motor_position_information mc_motor_position_information;
MC_current_information mc_current_informtarion;
BMS_coulomb_counts bms_coulomb_counts;

/*
 * Constant definitions
 */
 // TODO some of these values need to be calibrated once hardware is installed
#define BRAKE_ACTIVE 600                // Threshold for brake pedal active
#define MIN_ACCELERATOR_PEDAL_1 1850     // Low accelerator implausibility threshold
#define START_ACCELERATOR_PEDAL_1 2050   // Position to start acceleration
#define END_ACCELERATOR_PEDAL_1 2400     // Position to max out acceleration
#define MAX_ACCELERATOR_PEDAL_1 2500    // High accelerator implausibility threshold
#define MIN_ACCELERATOR_PEDAL_2 2250    // Low accelerator implausibility threshold
#define START_ACCELERATOR_PEDAL_2 2050  // Position to start acceleration
#define END_ACCELERATOR_PEDAL_2 1700    // Position to max out acceleration
#define MAX_ACCELERATOR_PEDAL_2 1590    // High accelerator implausibility threshold
#define MIN_HV_VOLTAGE 500              // Volts in V * 0.1 - Used to check if Accumulator is energized
#define BMS_HIGH 134                    // ~3V on BMS_OK line
#define IMD_HIGH 134                    // ~3V on OKHS line
#define SHUTDOWN_B_HIGH 530             // ~5V on SHUTDOWN_B line
#define SHUTDOWN_C_HIGH 350             // ~5V on SHUTDOWN_C line
#define SHUTDOWN_D_HIGH 350             // ~5V on SHUTDOWN_D line ??????
#define SHUTDOWN_E_HIGH 350             // ~5V on SHUTDOWN_E line
#define SHUTDOWN_F_HIGH 350             // ~5V on SHUTDOWN_F line
#define FAN_1_DUTY_CYCLE 127            // TODO: figure out correct duty cycle (0 = 0%, 255 = 100%)
#define FAN_2_DUTY_CYCLE 127            // TODO: figure out correct duty cycle (0 = 0%, 255 = 100%)
#define BMS_HIGH_BATTERY_TEMPERATURE 50 // TODO: figure out correct value
#define GLV_VOLTAGE_MULTIPLIER 5.5963   // TODO: calibrate this constant
#define MIN_RPM_FOR_REGEN 100           // TODO: calibrate this constant
#define START_ACCEL1_PEDAL_WITH_REGEN 190  // TODO: calibrate this constant
#define START_ACCEL2_PEDAL_WITH_REGEN 3890 // TODO: calibrate this constant
#define START_BRAKE_PEDAL_WITH_REGEN 450   // TODO: calibrate this constant
#define END_BRAKE_PEDAL_WITH_REGEN 1000    // TODO: calibrate this constant
#define ALPHA 0.9772                    // parameter for the sowftware filter used on ADC pedal channels
#define EXPANDER_SPI_SPEED 9000000      // max SPI clock frequency for MCP23S17 is 10MHz in ideal conditions
#define ADC_SPI_SPEED 1800000           // max SPI clokc frequency for MCP3208 is 2MHz in ideal conditions
#define TORQUE_ADJUSTMENT_VOLTAGE 3.5242   //
#define MAX_POSSIBLE_TORQUE 1600        //

/*
 * Timers
 */
Metro timer_bms_imd_print_fault = Metro(500);
Metro timer_btn_restart_inverter = Metro(100);
Metro timer_btn_mode = Metro(100);
Metro timer_btn_start = Metro(100);
Metro timer_debug = Metro(200);
Metro timer_debug_raw_torque = Metro(200);
Metro timer_debug_torque = Metro(200);
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable
Metro timer_led_mode_blink_fast = Metro(150, 1);
Metro timer_led_mode_blink_slow = Metro(400, 1);
Metro timer_led_start_blink_fast = Metro(150);
Metro timer_led_start_blink_slow = Metro(400);
Metro timer_motor_controller_send = Metro(50);
Metro timer_ready_sound = Metro(2000); // Time to play RTD sound
Metro timer_can_update = Metro(100);
Metro timer_bms_print_fault = Metro(500);
Metro timer_imd_print_fault = Metro(500);
Metro timer_restart_inverter = Metro(500, 1); // Allow the FCU to restart the inverter
Metro timer_status_send = Metro(100);

/*
 * Variables to store filtered values from ADC channels
 */
float filtered_accel1_reading = 0;
float filtered_accel2_reading = 0;
float filtered_brake_reading = 0;
float filtered_glv_reading = 0;

bool btn_start_reading = true;
bool btn_mode_reading = true;
bool btn_restart_inverter_reading = true;
bool imd_faulting = false;
bool inverter_restart = false; // True when restarting the inverter
bool btn_start_debounced = false;
bool btn_start_debouncing = false;
bool btn_start_pressed = false;
bool btn_mode_debouncing = false;
bool btn_mode_pressed = true;
bool btn_restart_inverter_debouncing = false;
bool btn_restart_inverter_pressed = false;
bool debug = false;
bool led_mode_active = false;
bool led_start_active = false;
uint8_t torque_mode = 0;
uint8_t led_mode_type = 0;
uint8_t led_start_type = 0; // 0 for off, 1 for steady, 2 for fast blink, 3 for slow blink

uint16_t MAX_TORQUE = MAX_POSSIBLE_TORQUE; // Torque in Nm * 10
int16_t MAX_REGEN_TORQUE = 0;
int16_t MAX_ACCEL_REGEN = 0;
int16_t MAX_BRAKE_REGEN = 0;
uint16_t dash_values = 0;
uint32_t total_charge_amount = 0;
uint32_t total_discharge_amount = 0;

static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
ADC_SPI ADC(ADC_CS, ADC_SPI_SPEED);
MCP23S17 EXPANDER(0, EXPANDER_CS, EXPANDER_SPI_SPEED);
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
    EXPANDER.pinMode(EXPANDER_LED_MC_ERR, OUTPUT);
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

    delay(500);

    Serial.println("CAN system and serial communication initialized");

    set_state(MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
    digitalWrite(SOFTWARE_SHUTDOWN_RELAY, HIGH);
    digitalWrite(SSR_INVERTER, HIGH);
    digitalWrite(PUMP_CTRL, HIGH);
    //analogWrite(FAN_1, FAN_1_DUTY_CYCLE);
    analogWrite(FAN_2, FAN_2_DUTY_CYCLE);
    mcu_status.set_bms_ok_high(true);
    mcu_status.set_imd_okhs_high(true);
    mcu_status.set_inverter_powered(true);
}

void loop() {

    read_pedal_values();
    read_dashboard_buttons();
    set_dashboard_leds();

    /*
     * Send state over CAN
     */
    if (timer_can_update.check()) {

        // Update the status values
        read_status_values();

        // Send Main Control Unit status message
        mcu_status.write(tx_msg.buf);
        tx_msg.id = ID_MCU_STATUS;
        tx_msg.len = sizeof(CAN_message_mcu_status_t);
        CAN.write(tx_msg);

        // Update the pedal readings to send over CAN
        mcu_pedal_readings.set_accelerator_pedal_raw_1(filtered_accel1_reading);
        mcu_pedal_readings.set_accelerator_pedal_raw_2(filtered_accel2_reading);
        mcu_pedal_readings.set_brake_pedal_raw(filtered_brake_reading);

        // Send Main Control Unit pedal reading message
        mcu_pedal_readings.write(tx_msg.buf);
        tx_msg.id = ID_MCU_PEDAL_READINGS;
        tx_msg.len = sizeof(CAN_message_mcu_pedal_readings_t);
        CAN.write(tx_msg);

        // Send couloumb counting information
        bms_coulomb_counts.set_total_charge(total_charge_amount);
        bms_coulomb_counts.set_total_discharge(total_discharge_amount);
        tx_msg.id = ID_BMS_COULOMB_COUNTS;
        tx_msg.len = sizeof(CAN_message_bms_coulomb_counts_t);
        CAN.write(tx_msg);
    }

    /*
     * Finish restarting the inverter when timer expires
     */
    if (timer_restart_inverter.check() && inverter_restart) {
        inverter_restart = false;
        digitalWrite(SSR_INVERTER, HIGH);
        mcu_status.set_inverter_powered(true);
    }

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
                //set_state(MCU_STATE_ENABLING_INVERTER);
                set_state(MCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
            }
        }
        break;

        case MCU_STATE_WAITING_READY_TO_DRIVE_SOUND:

        EXPANDER.digitalWrite(EXPANDER_READY_SOUND, HIGH);

        if (timer_ready_sound.check()) {
            //EXPANDER.digitalWrite(EXPANDER_READY_SOUND, LOW);
            set_state(MCU_STATE_ENABLING_INVERTER);
        }
        break;

        case MCU_STATE_ENABLING_INVERTER:
        if (timer_inverter_enable.check()) {
            set_state(MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        }
        break;

        // case MCU_STATE_WAITING_READY_TO_DRIVE_SOUND:

        // EXPANDER.digitalWrite(EXPANDER_READY_SOUND, HIGH);

        // if (timer_ready_sound.check()) {
        //     set_state(MCU_STATE_READY_TO_DRIVE);
        // }
        // break;

        case MCU_STATE_READY_TO_DRIVE:
        if (timer_motor_controller_send.check()) {
            MC_command_message mc_command_message = MC_command_message(0, 0, 1, 1, 0, 0);
            //read_pedal_values();

            // Check for accelerator implausibility FSAE EV2.3.10
            mcu_pedal_readings.set_accelerator_implausibility(false);
            if (mcu_pedal_readings.get_accelerator_pedal_raw_1() < MIN_ACCELERATOR_PEDAL_1 || mcu_pedal_readings.get_accelerator_pedal_raw_1() > MAX_ACCELERATOR_PEDAL_1) {
                mcu_pedal_readings.set_accelerator_implausibility(true);
            }
            if (mcu_pedal_readings.get_accelerator_pedal_raw_2() > MIN_ACCELERATOR_PEDAL_2 || mcu_pedal_readings.get_accelerator_pedal_raw_2() < MAX_ACCELERATOR_PEDAL_2) {
                mcu_pedal_readings.set_accelerator_implausibility(true);
            }

            int calculated_torque = calculate_torque();

            // FSAE EV2.5 APPS / Brake Pedal Plausibility Check
            if (mcu_pedal_readings.get_brake_implausibility() && calculated_torque < (MAX_TORQUE / 20)) {
                mcu_pedal_readings.set_brake_implausibility(false); // Clear implausibility
            }
            // if (mcu_pedal_readings.get_brake_pedal_active() && calculated_torque > (MAX_TORQUE / 4)) {
            //     mcu_pedal_readings.set_brake_implausibility(true);
            // }

            if (mcu_pedal_readings.get_brake_implausibility() || mcu_pedal_readings.get_accelerator_implausibility()) {
                // Implausibility exists, command 0 torque
                calculated_torque = 0;
            }

            // FSAE FMEA specifications // if BMS or IMD are faulting, set torque to 0
            if (!mcu_status.get_bms_ok_high()) {
                calculated_torque = 0;
            }

            if (!mcu_status.get_imd_okhs_high()) {
                calculated_torque = 0;
            }

            if (debug && timer_debug_torque.check()) {
                Serial.print("MCU REQUESTED TORQUE: ");
                Serial.println(calculated_torque);
                Serial.print("MCU IMPLAUS ACCEL: ");
                Serial.println(mcu_pedal_readings.get_accelerator_implausibility());
                Serial.print("MCU IMPLAUS BRAKE: ");
                Serial.println(mcu_pedal_readings.get_brake_implausibility());
            }

            // Serial.print("RPM: ");
            // Serial.println(mc_motor_position_information.get_motor_speed());
            //Serial.println(calculated_torque);

            mc_command_message.set_torque_command(0);

            mc_command_message.write(tx_msg.buf);
            tx_msg.id = ID_MC_COMMAND_MESSAGE;
            tx_msg.len = 8;
            CAN.write(tx_msg);
        }
        break;

    }

    /*
     * Send a message to the Motor Controller over CAN when vehicle is not ready to drive
     */
    if (mcu_status.get_state() < MCU_STATE_READY_TO_DRIVE && timer_motor_controller_send.check()) {
        MC_command_message mc_command_message = MC_command_message(0, 0, 1, 0, 0, 0);

        // if (mcu_status.get_state() >= MCU_STATE_ENABLING_INVERTER) {
        //      mc_command_message.set_inverter_enable(true);
        // }

        mc_command_message.write(tx_msg.buf);
        tx_msg.id = ID_MC_COMMAND_MESSAGE;
        tx_msg.len = 8;
        CAN.write(tx_msg);
    }
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
                //set_state(MCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
                set_state(MCU_STATE_READY_TO_DRIVE);
            }
            if (!mc_internal_states.get_inverter_enable_state() && mcu_status.get_state() == MCU_STATE_READY_TO_DRIVE) {
                set_state(MCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
            }
        }

        if (rx_msg.id == ID_MC_MOTOR_POSITION_INFORMATION) {
            mc_motor_position_information.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_STATUS) {
            bms_status.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_TEMPERATURES) {
            bms_temperatures.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_VOLTAGES) {
            bms_voltages.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_MC_CURRENT_INFORMATION) {
            if (mcu_status.get_state() == MCU_STATE_READY_TO_DRIVE) {
                mc_current_informtarion.load(rx_msg.buf);
                update_couloumb_count();
            }
        }
    }

}

void reset_inverter() {
    inverter_restart = true;
    digitalWrite(SSR_INVERTER, LOW);
    timer_restart_inverter.reset();
    mcu_status.set_inverter_powered(false);
    Serial.println("INVERTER RESET");
}

/*
 * Read values of sensors
 */
void read_pedal_values() {

    /*
     * Filter ADC readings
     */
    int accel1 = ADC.read_adc(ADC_ACCEL_1_CHANNEL);
    int accel2 = ADC.read_adc(ADC_ACCEL_2_CHANNEL);
    int brake  = ADC.read_adc(ADC_BRAKE_CHANNEL);

    filtered_accel1_reading = ALPHA * filtered_accel1_reading + (1 - ALPHA) * accel1;
    filtered_accel2_reading = ALPHA * filtered_accel2_reading + (1 - ALPHA) * accel2;
    filtered_brake_reading  = ALPHA * filtered_brake_reading  + (1 - ALPHA) * brake;
    // Serial.print("ACCEL 1: "); Serial.println(filtered_accel1_reading);
    // Serial.print("ACCEL 2: "); Serial.println(filtered_accel2_reading);
    // Serial.print("BRAKE: "); Serial.println(filtered_brake_reading);

    Serial.print(accel1); Serial.print(" ");
    Serial.print(filtered_accel1_reading); Serial.print(" ");
    Serial.print(accel2); Serial.print(" ");
    Serial.print(filtered_accel2_reading); Serial.print(" ");
    Serial.print(brake); Serial.print(" ");
    Serial.print(filtered_brake_reading); Serial.print(" ");
    Serial.println(micros());


    //Serial.println(ADC.read_adc(ADC_ACCEL_1_CHANNEL));

    // set the brake pedal active flag if the median reading is above the threshold
    mcu_pedal_readings.set_brake_pedal_active(filtered_brake_reading >= BRAKE_ACTIVE);
    digitalWrite(SSR_BRAKE_LIGHT, mcu_pedal_readings.get_brake_pedal_active());

    /*
     * Print values for debugging
     */
    if (debug && timer_debug.check()) {
        Serial.print("MCU PEDAL ACCEL 1: ");
        Serial.println(mcu_pedal_readings.get_accelerator_pedal_raw_1());
        Serial.print("MCU PEDAL ACCEL 2: ");
        Serial.println(mcu_pedal_readings.get_accelerator_pedal_raw_2());
        Serial.print("MCU PEDAL BRAKE: ");
        Serial.println(mcu_pedal_readings.get_brake_pedal_raw());
        Serial.print("MCU BRAKE ACT: ");
        Serial.println(mcu_pedal_readings.get_brake_pedal_active());
        Serial.print("MCU STATE: ");
        Serial.println(mcu_status.get_state());
    }
}

void read_status_values() {

    /*
     * Filter ADC readings of GLV voltage
     */
    //filtered_glv_reading += ALPHA * filtered_glv_reading + (1 - ALPHA) * ADC.read_adc(ADC_12V_SUPPLY_CHANNEL);

    mcu_status.set_glv_battery_voltage(ADC.read_adc(ADC_12V_SUPPLY_CHANNEL) * GLV_VOLTAGE_MULTIPLIER); // convert GLV voltage and to send it over CAN


    /*
     * Check for BMS fault
     */
    if (ADC.read_adc(ADC_BMS_OK_CHANNEL) > BMS_HIGH) {
        mcu_status.set_bms_ok_high(true);
    } else {
        mcu_status.set_bms_ok_high(false);
        if (timer_bms_print_fault.check()) {
            Serial.println("BMS fault detected");
        }
    }

    /*
     * Check for IMD fault
     */
    if (ADC.read_adc(ADC_OKHS_CHANNEL) > IMD_HIGH) {
        mcu_status.set_imd_okhs_high(true);
    } else {
        mcu_status.set_imd_okhs_high(false);
        if (timer_imd_print_fault.check()) {
            Serial.println("IMD fault detected");
        }
    }

    /*
     * Measure shutdown circuits' voltages
     */
     mcu_status.set_shutdown_b_above_threshold(analogRead(SENSE_SHUTDOWN_B) > SHUTDOWN_B_HIGH);
     mcu_status.set_shutdown_c_above_threshold(analogRead(SENSE_SHUTDOWN_C) > SHUTDOWN_C_HIGH);
     mcu_status.set_shutdown_d_above_threshold(ADC.read_adc(ADC_SHUTDOWN_D_READ_CHANNEL) > SHUTDOWN_D_HIGH);
     mcu_status.set_shutdown_e_above_threshold(analogRead(SENSE_SHUTDOWN_E) > SHUTDOWN_E_HIGH);
     mcu_status.set_shutdown_f_above_threshold(analogRead(SENSE_SHUTDOWN_F) > SHUTDOWN_F_HIGH);

     /*
      * Measure the temperature from on-board thermistors
      */
     mcu_status.set_temperature(ADC.read_adc(ADC_TEMPSENSE_CHANNEL) * 100); // send temperatures in 0.01 C
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
            timer_led_mode_blink_fast.reset();
            if (debug) {
                Serial.println("MCU Setting Mode LED solid on");
            }
        } else if (type == 2) {
            // timer_led_mode_blink_fast.reset();
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
    }
    if (new_state == MCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
        set_start_led(2);
    }
    if (new_state == MCU_STATE_ENABLING_INVERTER) {
        EXPANDER.digitalWrite(EXPANDER_READY_SOUND, LOW);
        // states are switched to fix the RTDS not being loud enough
        Serial.println("RTDS deactivated");

        set_start_led(1);
        Serial.println("MCU Enabling inverter");
        MC_command_message mc_command_message = MC_command_message(0, 0, 1, 1, 0, 0);
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
        //Serial.println("Inverter enabled");
        Serial.println("RTDS enabled");
    }
    if (new_state == MCU_STATE_READY_TO_DRIVE) {
        //EXPANDER.digitalWrite(EXPANDER_READY_SOUND, LOW);

        Serial.println("Inverter enabled");
        //Serial.println("RTDS deactivated");
        Serial.println("Ready to drive");
    }
}

int calculate_torque() {
    int calculated_torque = 0;

    //if (!mcu_pedal_readings.get_accelerator_implausibility()) {
        int torque1 = map(round(filtered_accel1_reading), START_ACCELERATOR_PEDAL_1, END_ACCELERATOR_PEDAL_1, 0, MAX_TORQUE);
        int torque2 = map(round(filtered_accel2_reading), START_ACCELERATOR_PEDAL_2, END_ACCELERATOR_PEDAL_2, 0, MAX_TORQUE);

        // torque values are greater than the max possible value, set them to max
        if (torque1 > MAX_TORQUE) {
            torque1 = MAX_TORQUE;
        }
        if (torque2 > MAX_TORQUE) {
            torque2 = MAX_TORQUE;
        }
        // compare torques to check for accelerator implausibility
        if (0)/*abs(torque1 - torque2) * 100 / MAX_TORQUE > 10) */{
            mcu_pedal_readings.set_accelerator_implausibility(true);
            Serial.println("ACCEL IMPLAUSIBILITY: COMPARISON FAILED");
        } else {
            calculated_torque = (torque1 + torque2) / 2; //min(torque1, torque2);

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
        }
    //}

    return calculated_torque;
}

void read_dashboard_buttons() {

    int dash_reading = EXPANDER.digitalRead();

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
            torque_mode = (torque_mode + 1) % 3;
            if (torque_mode == 0) {
                set_mode_led(0);
                // 40
                MAX_TORQUE = 400;
                MAX_ACCEL_REGEN = 0;
                MAX_BRAKE_REGEN = 0;
            } else if (torque_mode == 1) {
                // blink 80
                MAX_TORQUE = 800;
                set_mode_led(1);
                MAX_ACCEL_REGEN = 0;
                MAX_BRAKE_REGEN = -400;
            } else if (torque_mode == 2) {
                // solid 160
                MAX_TORQUE = 1600;
                set_mode_led(2);
                MAX_ACCEL_REGEN = -100;
                MAX_BRAKE_REGEN = -400;
            } //else if (torque_mode == 3) {
            //     set_mode_led(3);
            //     MAX_ACCEL_REGEN = -400;
            //     MAX_BRAKE_REGEN = 0;
            // }
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
        if (btn_restart_inverter_pressed) {
            reset_inverter();
        }
    }
}

void set_dashboard_leds() {

    /*
     * Set torque mode led
     */
    if ((led_mode_type == 1 && timer_led_mode_blink_fast.check())) { //|| (led_mode_type == 3 && timer_led_mode_blink_slow.check())) {
        if (led_mode_active) {
            dash_values |= (1 << EXPANDER_LED_MODE);
        }
        else {
            dash_values &= ~(1 << EXPANDER_LED_MODE);
        }

        led_mode_active = !led_mode_active;
    }
    if (led_mode_type == 0) {
        led_mode_active = led_mode_type;

        if (led_mode_active) {
            dash_values |= (1 << EXPANDER_LED_MODE);
        }
        else {
            dash_values &= ~(1 << EXPANDER_LED_MODE);
        }
    }

    if (led_mode_type == 2) {
        led_mode_active = 1;

        if (led_mode_active) {
            dash_values |= (1 << EXPANDER_LED_MODE);
        }
        else {
            dash_values &= ~(1 << EXPANDER_LED_MODE);
        }
    }

    /*
     * Set start led
     */
    if ((led_start_type == 2 && timer_led_start_blink_fast.check()) || (led_start_type == 3 && timer_led_start_blink_slow.check())) {
        if (led_start_active) {
            dash_values |= (1 << EXPANDER_LED_START);
        }
        else {
            dash_values &= ~(1 << EXPANDER_LED_START);
        }
        led_start_active = !led_start_active;
    }
    if (led_start_type < 2) {
        led_start_active = led_start_type;
        if (led_start_active) {
            dash_values |= (1 << EXPANDER_LED_START);
        }
        else {
            dash_values &= ~(1 << EXPANDER_LED_START);
        }
    }

    /*
     * Set BMS and IMD leds
     */
    if (!mcu_status.get_bms_ok_high()) {
        dash_values |= (1 << EXPANDER_LED_BMS);
    }
    else {
        dash_values &= ~(1 << EXPANDER_LED_BMS);
    }
    if (!mcu_status.get_imd_okhs_high()) {
        dash_values |= (1 << EXPANDER_LED_IMD);
    }
    else {
        dash_values &= ~(1 << EXPANDER_LED_IMD);
    }

    EXPANDER.digitalWrite(dash_values);
}

// NOT TESTED YET
int calculate_torque_with_regen() {
    if (mc_motor_position_information.get_motor_speed() < MIN_RPM_FOR_REGEN) {
        MAX_ACCEL_REGEN = 0;
        MAX_BRAKE_REGEN = 0;
    }

    int calculated_torque = 0;
    double lowest_cell_voltage = bms_voltages.get_low() / 10000.0;

    if (lowest_cell_voltage < TORQUE_ADJUSTMENT_VOLTAGE) { // TODO: adjust this
        MAX_TORQUE = map(lowest_cell_voltage, 0, TORQUE_ADJUSTMENT_VOLTAGE, 0, MAX_POSSIBLE_TORQUE);
    }

    int torque1 = map(round(filtered_accel1_reading), START_ACCEL1_PEDAL_WITH_REGEN, END_ACCELERATOR_PEDAL_1, MAX_ACCEL_REGEN, MAX_TORQUE);
    int torque2 = map(round(filtered_accel2_reading), START_ACCEL2_PEDAL_WITH_REGEN, END_ACCELERATOR_PEDAL_2, MAX_ACCEL_REGEN, MAX_TORQUE);
    int torque3 = map(round(filtered_brake_reading), START_BRAKE_PEDAL_WITH_REGEN, END_BRAKE_PEDAL_WITH_REGEN, 0, MAX_BRAKE_REGEN);

    if (torque1 > MAX_TORQUE) {
        torque1 = MAX_TORQUE;
    }
    if (torque2 > MAX_TORQUE) {
        torque2 = MAX_TORQUE;
    }
    if (torque3 > 0) {
        torque3 = 0;
    }
    if (torque1 < MAX_ACCEL_REGEN) {
        torque1 = MAX_ACCEL_REGEN;
    }
    if (torque2 < MAX_ACCEL_REGEN) {
        torque2 = MAX_ACCEL_REGEN;
    }
    if (torque3 < MAX_BRAKE_REGEN) {
        torque3 = MAX_BRAKE_REGEN;
    }

    //Serial.println((torque1 + torque2) / 2);
    // compare torques to check for accelerator implausibility
    if (0){//abs(torque1 - torque2) * 100 / MAX_TORQUE > 10) {
        mcu_pedal_readings.set_accelerator_implausibility(true);
        Serial.println("ACCEL IMPLAUSIBILITY: COMPARISON FAILED");
    } else {
        calculated_torque = (torque1 + torque2) / 2 + torque3;

        if (debug && timer_debug_raw_torque.check()) {
            Serial.print("TORQUE REQUEST DELTA PERCENT: "); // Print the % difference between the 2 accelerator sensor requests
            Serial.println(abs(torque1 - torque2) / (double) MAX_TORQUE * 100);
            Serial.print("MCU RAW TORQUE: ");
            Serial.println(calculated_torque);
        }
        if (calculated_torque > MAX_TORQUE) {
            calculated_torque = MAX_TORQUE;
        }
        if (calculated_torque < (MAX_ACCEL_REGEN + MAX_BRAKE_REGEN)) {
            calculated_torque = (MAX_ACCEL_REGEN + MAX_BRAKE_REGEN);
        }
    }

    return calculated_torque;
}

void update_couloumb_count() {
    int new_current = mc_current_informtarion.get_dc_bus_current() * 10; // get current in Amps * 100
    if (new_current > 0) {
        total_discharge_amount += new_current; //
    } else {
        total_charge_amount -= new_current;
    }
}
