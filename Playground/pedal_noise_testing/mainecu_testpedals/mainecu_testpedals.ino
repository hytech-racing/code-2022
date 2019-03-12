/*
 * HyTech 2019 Vehicle Front Control Unit
 * Interface with dashboard lights, buttons, and buzzer.
 * Read pedal sensor values and communicate with motor controller.
 * Configured for Front Control Unit rev7
 */
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <ADC_SPI.h>
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Metro.h>
#include <Wire.h>

/*
 * Pin definitions
 */
#define ADC_ACCEL_1_CHANNEL 1
#define ADC_ACCEL_2_CHANNEL 2
#define ADC_BRAKE_CHANNEL 0
#define ADC_SPI_CS 9
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

/*
 * Timers
 */
Metro timer_accelerometer = Metro(100);
Metro timer_bms_imd_print_fault = Metro(500);
Metro timer_btn_cycle = Metro(10);
Metro timer_btn_mode = Metro(10);
Metro timer_btn_start = Metro(10);
Metro timer_debug = Metro(200);
Metro timer_debug_raw_torque = Metro(200);
Metro timer_debug_torque = Metro(200);
Metro timer_ramp_torque = Metro(100);
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable
Metro timer_led_mode_blink_fast = Metro(150);
Metro timer_led_mode_blink_slow = Metro(400);
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
FCU_accelerometer_values fcu_accelerometer_values;

bool btn_start_debouncing = false;
uint8_t btn_start_new = 0;
bool btn_start_pressed = false;
bool btn_mode_debouncing = false;
uint8_t btn_mode_new = 0;
bool btn_mode_pressed = true;
bool btn_cycle_debouncing = false;
bool btn_cycle_pressed = false;
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

ADC_SPI ADC(ADC_SPI_CS);
FlexCAN CAN(500000);
static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void setup() {
    pinMode(A9, OUTPUT);
    analogWriteFrequency(A9, 80);
    analogWrite(A9, 128);
    //digitalWrite(A9, 1);
    Serial.begin(115200); // Init serial for PC communication
    CAN.begin(); // Init CAN for vehicle communication

    /* Configure CAN rx interrupt */
    interrupts();
    NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);
    attachInterruptVector(IRQ_CAN_MESSAGE,parse_can_message);
    FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;
    /* Configure CAN rx interrupt */

    delay(100);
    Serial.println("CAN system and serial communication initialized");

}

double max_err = 0;
void loop() {

        if (timer_motor_controller_send.check()) {
            MC_command_message mc_command_message = MC_command_message(0, 0, 0, 1, 0, 0);
            read_values(); // Read new sensor values
            
            double err = abs((double) abs(fcu_readings.get_accelerator_pedal_raw_1() - 2048) - abs(fcu_readings.get_accelerator_pedal_raw_2() - 2048)) / 4095;
            if (err > max_err) {
              max_err = err;
            }
            Serial.print("ERR: ");
            Serial.println(err,5);
            Serial.print("MAX ERR: ");
            Serial.println(max_err,5);
                
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
        Serial.print("FCU PEDAL ACCEL 1: ");
        Serial.println(fcu_readings.get_accelerator_pedal_raw_1());
        Serial.print("FCU PEDAL ACCEL 2: ");
        Serial.println(fcu_readings.get_accelerator_pedal_raw_2());
        Serial.print("FCU PEDAL BRAKE: ");
        Serial.println(fcu_readings.get_brake_pedal_raw());
    }
    // TODO calculate temperature
}

void parse_can_message() {
    while (CAN.read(rx_msg)) {
        if (rx_msg.id == ID_RCU_STATUS) {
            rcu_status.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_MC_VOLTAGE_INFORMATION) {
            MC_voltage_information mc_voltage_information = MC_voltage_information(rx_msg.buf);
            if (mc_voltage_information.get_dc_bus_voltage() >= MIN_HV_VOLTAGE && fcu_status.get_state() == FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
                //set_state(FCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
            }
            if (mc_voltage_information.get_dc_bus_voltage() < MIN_HV_VOLTAGE && fcu_status.get_state() > FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
                //set_state(FCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
            }
        }

        if (rx_msg.id == ID_MC_INTERNAL_STATES) {
            MC_internal_states mc_internal_states = MC_internal_states(rx_msg.buf);
            if (mc_internal_states.get_inverter_enable_state() && fcu_status.get_state() == FCU_STATE_ENABLING_INVERTER) {
                //set_state(FCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
            }
        }

        if (rx_msg.id == ID_BMS_STATUS) {
            bms_status.load(rx_msg.buf);
        }
    }
}

