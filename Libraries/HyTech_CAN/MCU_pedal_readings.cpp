/*
 * MCU_pedal_readings.cpp - CAN message parser: Main Control Unit pedal readings message
 * Created Feb, 2019.
 */

#include "HyTech_CAN.h"

/*  Blank Constructor for MCU_pedal_readings
 *
 * Used to initialize instance of MCU_pedal_readings with no data
 */

MCU_pedal_readings::MCU_pedal_readings() {
    message = {};
}

/* Constructor for MCU_pedal_readings using a buffer
 *
 * Used to initialize instance of MCU_pedal_readings with data
 * that's in an 8xbyte array (typically msg.buf)
 *
 * Param - Pass in buffer you are trying to initialize data from
 */

MCU_pedal_readings::MCU_pedal_readings(uint8_t buf[8]) {
    load(buf);
}

/* Constructor for MCU_pedal_readings
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param (uint16_t) - Accelerator Pedal Raw Value 1
 *     - Raw voltage readings from accelerator pedal sensor 1
 * Param (uint16_t) - Accelerator Pedal Raw Value 2
 *     - Raw voltage readings from accelerator pedal sensor 2
 * Param (uint16_t) - Brake Pedal Raw Value
 *     - Raw voltage readings from brake pedal sensor
 * Param (uint8_t) - Pedal Flags
 *     - Indicators for accelerator/brake implausibility and active brake pedal
 * Param (uint8_t) - Torque Map Mode
 *     - Torque map mode selection
 */

MCU_pedal_readings::MCU_pedal_readings(uint16_t accelerator_pedal_raw_1, uint16_t accelerator_pedal_raw_2, uint16_t brake_pedal_raw, uint8_t pedal_flags, uint8_t torque_map_mode) {
    set_accelerator_pedal_raw_1(accelerator_pedal_raw_1);
    set_accelerator_pedal_raw_2(accelerator_pedal_raw_2);
    set_brake_pedal_raw(brake_pedal_raw);
    set_pedal_flags(pedal_flags);
    set_torque_map_mode(torque_map_mode);
}

/* Load from buffer & write to variable instance
 *
 * Used to copy data from msg variable in
 * microcontroller code to instance variable
 *
 * Param - Pass in buffer you are trying to read from
 * Example: curMCU_pedal_readings.load(msg.buf);
 */

void MCU_pedal_readings::load(uint8_t buf[8]) {
    message = {};

    memcpy(&(message.accelerator_pedal_raw_1), &buf[0], sizeof(uint16_t));
    memcpy(&(message.accelerator_pedal_raw_2), &buf[2], sizeof(uint16_t));
    memcpy(&(message.brake_pedal_raw), &buf[4], sizeof(uint16_t));
    memcpy(&(message.pedal_flags), &buf[6], sizeof(uint8_t));
    memcpy(&(message.torque_map_mode), &buf[7], sizeof(uint8_t));

}

/* Write to buffer
 *
 * Used to copy data from instance of this class
 * to msg variable in microcontroller code
 *
 * Param - Pass in buffer you are trying to modify
 * Example: curMCU_pedal_readings.write(msg.buf);
 */

void MCU_pedal_readings::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.accelerator_pedal_raw_1), sizeof(uint16_t));
    memcpy(&buf[2], &(message.accelerator_pedal_raw_2), sizeof(uint16_t));
    memcpy(&buf[4], &(message.brake_pedal_raw), sizeof(uint16_t));
    memcpy(&buf[6], &(message.pedal_flags), sizeof(uint8_t));
    memcpy(&buf[7], &(message.torque_map_mode), sizeof(uint8_t));

}

/* Get functions
 *
 * Used to retrieve values stored in this CAN_message_t
 */

uint16_t MCU_pedal_readings::get_accelerator_pedal_raw_1() {
    return message.accelerator_pedal_raw_1;
}

uint16_t MCU_pedal_readings::get_accelerator_pedal_raw_2() {
    return message.accelerator_pedal_raw_2;
}

uint16_t MCU_pedal_readings::get_brake_pedal_raw() {
    return message.brake_pedal_raw;
}

uint8_t MCU_pedal_readings::get_pedal_flags() {
    return message.pedal_flags;
}

bool MCU_pedal_readings::get_accelerator_implausibility() {
    return message.pedal_flags & 0x01;
}

bool MCU_pedal_readings::get_brake_implausibility() {
    return (message.pedal_flags & 0x02) >> 1;
}

bool MCU_pedal_readings::get_brake_pedal_active() {
    return (message.pedal_flags & 0x04) >> 2;
}

uint8_t MCU_pedal_readings::get_torque_map_mode() {
    return message.torque_map_mode;
}

/* Set functions
 *
 * Used to replace values in this CAN_message_t
 *
 * Param (uint16_t) - Variable to replace old data
 */

void MCU_pedal_readings::set_accelerator_pedal_raw_1(uint16_t accelerator_pedal_raw_1) {
    message.accelerator_pedal_raw_1 = accelerator_pedal_raw_1;
}

void MCU_pedal_readings::set_accelerator_pedal_raw_2(uint16_t accelerator_pedal_raw_2) {
    message.accelerator_pedal_raw_2 = accelerator_pedal_raw_2;
}

void MCU_pedal_readings::set_brake_pedal_raw(uint16_t brake_pedal_raw) {
    message.brake_pedal_raw = brake_pedal_raw;
}

void MCU_pedal_readings::set_pedal_flags(uint8_t pedal_flags) {
    message.pedal_flags = pedal_flags;
}

void MCU_pedal_readings::set_accelerator_implausibility(bool accelerator_implausibility) {
    message.pedal_flags = (message.pedal_flags & 0xFE) | (accelerator_implausibility & 0x1);
}

void MCU_pedal_readings::set_brake_implausibility(bool brake_implausibility) {
    message.pedal_flags = (message.pedal_flags & 0xFD) | ((brake_implausibility & 0x1) << 1);
}

void MCU_pedal_readings::set_brake_pedal_active(bool brake_pedal_active) {
    message.pedal_flags = (message.pedal_flags & 0xFB) | ((brake_pedal_active & 0x1) << 2);
}

void MCU_pedal_readings::set_torque_map_mode(uint8_t torque_map_mode) {
    message.torque_map_mode = torque_map_mode;
}