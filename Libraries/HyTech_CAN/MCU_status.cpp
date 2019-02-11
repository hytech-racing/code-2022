/*
 * MCU_status.cpp - CAN message parser: Main Control Unit status message
 * Created Feb, 2019.
 */

#include "HyTech_CAN.h"

/*  Blank Constructor for MCU_status
 *
 * Used to initialize instance of MCU_status with no data
 */

MCU_status::MCU_status() {
    message = {};
}

/* Constructor for MCU_status using a buffer
 *
 * Used to initialize instance of MCU_status with data
 * that's in an 8xbyte array (typically msg.buf)
 *
 * Param - Pass in buffer you are trying to initialize data from
 */

MCU_status::MCU_status(uint8_t buf[8]) {
    load(buf);
}

/* Constructor for MCU_status
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param (uint8_t) - Board State
 *     - Board state value
 * Param (uint8_t) - Board Flags
 *     - Indicators for BMS OK High, IMD OKHS High, and shutdown circuit B/C/D voltage above threshold
 * Param (int16_t) - Temperature
 *     - Value from the board thermistor (in C) times 100
 * Param (uint16_t) - GLV Battery Voltage
 *     - Battery voltage reading (in Volts) times 1000
 */

MCU_status::MCU_status(uint8_t state, uint8_t flags, int16_t temperature, uint16_t glv_battery_voltage) {
    set_state(state);
    set_flags(flags);
    set_temperature(temperature);
    set_glv_battery_voltage(glv_battery_voltage);
}

/* Load from buffer & write to variable instance
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param - Pass in buffer you are trying to read from
 * Example: curMCU_status.load(msg.buf);
 */

void MCU_status::load(uint8_t buf[8]) {
    message = {};

    memcpy(&(message.state), &buf[0], sizeof(uint8_t));
    memcpy(&(message.flags), &buf[1], sizeof(uint8_t));
    memcpy(&(message.temperature), &buf[2], sizeof(int16_t));
    memcpy(&(message.glv_battery_voltage), &buf[4], sizeof(uint16_t));
}

/* Write to buffer
 *
 * Used to copy data from instance of this class
 * to msg variable in microcontroller code
 *
 * Param - Pass in buffer you are trying to modify
 * Example: curMCU_status.write(msg.buf);
 */

void MCU_status::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.state), sizeof(uint8_t));
    memcpy(&buf[1], &(message.flags), sizeof(uint8_t));
    memcpy(&buf[2], &(message.temperature), sizeof(int16_t));
    memcpy(&buf[4], &(message.glv_battery_voltage), sizeof(uint16_t));
}

/* Get functions
 *
 * Used to retrieve values stored in this CAN_message_t
 */


uint8_t MCU_status::get_state() {
    return message.state;
}

uint8_t MCU_status::get_flags() {
    return message.flags;
}

bool MCU_status::get_bms_ok_high() {
    return message.flags & 0x1;
}

bool MCU_status::get_imd_okhs_high() {
    return (message.flags & 0x2) >> 1;
}

bool MCU_status::get_inverter_powered() {
    return (message.flags & 0x4) >> 2;
}

bool MCU_status::get_shutdown_b_above_threshold() {
    return (message.flags & 0x8) >> 3;
}

bool MCU_status::get_shutdown_c_above_threshold() {
    return (message.flags & 0x10) >> 4;
}

bool MCU_status::get_shutdown_d_above_threshold() {
    return (message.flags & 0x20) >> 5
}

int16_t MCU_status::get_temperature() {
    return message.temperature;
}

uint16_t MCU_status::get_glv_battery_voltage() {
    return message.glv_battery_voltage;
}

/* Set functions
 *
 * Used to replace values in this CAN_message_t
 *
 * Param (uint16_t) - Variable to replace old data
 */

void MCU_status::set_state(uint8_t state) {
    message.state = state;
}

void MCU_status::set_flags(uint8_t flags) {
    message.flags = flags;
}

void MCU_status::set_bms_ok_high(bool bms_ok_high) {
    message.flags = (message.flags & 0xFE) | (bms_ok_high & 0x1);
}

void MCU_status::set_imd_okhs_high(bool imd_okhs_high) {
    message.flags = (message.flags & 0xFD) | ((imd_okhs_high & 0x1) << 1);
}

void MCU_status::set_inverter_powered(bool inverter_powered) {
    message.flags = (message.flags & 0xFB) | ((inverter_powered & 0x1) << 2);
}

void MCU_status::set_shutdown_b_above_threshold(bool shutdown_b_above_threshold) {
    message.flags = (message.flags & 0xF7) | ((shutdown_b_above_threshold & 0x1) << 3);
}

void MCU_status::set_shutdown_c_above_threshold(bool shutdown_c_above_threshold) {
    message.flags = (message.flags & 0xEF) | ((shutdown_c_above_threshold & 0x1) << 4);
}

void MCU_status::set_shutdown_d_above_threshold(bool shutdown_d_above_threshold) {
    message.flags = (message.flags & 0xDF) | ((shutdown_d_above_threshold & 0x1) << 5);
}

void MCU_status::set_temperature(int16_t temperature) {
    message.temperature = temperature;
}

void MCU_status::set_glv_battery_voltage(uint16_t glv_battery_voltage) {
    message.glv_battery_voltage = glv_battery_voltage;
}