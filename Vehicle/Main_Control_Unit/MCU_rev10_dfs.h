#pragma once

/*
 * Teensy Pin definitions
 */
#define BRAKE_LIGHT_CTRL 0
#define FRONT_LEFT_WHEEL 1
#define FRONT_RIGHT_WHEEL 2
#define FAN_1 5
#define FAN_2 6
#define BACK_LEFT_WHEEL 7
#define BACK_RIGHT_WHEEL 8
#define INVERTER_CTRL 9
#define ADC_CS 10

#define SHUTDOWN_B_READ A0
#define SHUTDOWN_C_READ A1
#define SHUTDOWN_D_READ A2
#define SHUTDOWN_E_READ A3
#define IMD_OK_READ A4
#define BMS_OK_READ A5
#define BSPD_OK_READ A6
#define SOFTWARE_OK_READ A7
// digital outputs
#define WATCHDOG_INPUT A8
#define TEENSY_OK A9

/*
 * ADC pin definitions
 */
#define ADC_BRAKE_1_CHANNEL 1
#define ADC_BRAKE_2_CHANNEL 0
#define ADC_ACCEL_1_CHANNEL 2
#define ADC_ACCEL_2_CHANNEL 3

/*
 * Shutdown read thresholds
 */

#define SHUTDOWN_HIGH 350 //teensy ADC reading when the shutdown lines are at 5V


/*
 * Other constants
 */
#define MIN_HV_VOLTAGE 600               // Volts in V * 0.1 - Used to check if Accumulator is energized

#define FAN_1_DUTY_CYCLE 255             // TODO: figure out correct duty cycle (0 = 0%, 255 = 100%)
#define FAN_2_DUTY_CYCLE 255             // TODO: figure out correct duty cycle (0 = 0%, 255 = 100%)

#define BMS_HIGH_BATTERY_TEMPERATURE 50  // TODO: figure out correct value
#define GLV_VOLTAGE_MULTIPLIER 5.5963    // TODO: calibrate this constant

#define ALPHA 0.9772                     // parameter for the sowftware filter used on ADC pedal channels
#define ADC_SPI_SPEED 1800000            // max SPI clokc frequency for MCP3208 is 2MHz in ideal conditions
#define TORQUE_ADJUSTMENT_VOLTAGE 3.5242 

#define BMS_HEARTBEAT_TIMEOUT 3000
#define DASH_HEARTBEAT_TIMEOUT 1000
#define TIMER_SOFTWARE_ENABLE 100

// this is just for reference
#define MAX_POSSIBLE_TORQUE 1200         


#define NUM_TEETH 24 //CHANGE THIS FOR #OF TEETH PER REVOLUTION
#define WHEEL_CIRCUMFERENCE 1.300619 //CIRCUMFERENCE OF WHEEL IN METERS
#define TIME_OUT 500000
