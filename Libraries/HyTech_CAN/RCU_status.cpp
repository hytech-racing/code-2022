/*
 * RCU_status.cpp - CAN message parser: Power Control Unit status message
 * Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech_CAN.h"

RCU_status::RCU_status() {
    message = {};
}

RCU_status::RCU_status(uint8_t buf[8]) {
    load(buf);
}

RCU_status::RCU_status(uint8_t state, uint8_t flags, uint16_t glv_battery_voltage, int16_t temperature) {
    set_state(state);
    set_flags(flags);
    set_glv_battery_voltage(glv_battery_voltage);
    set_temperature(temperature);
}

void RCU_status::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.state), &buf[0], sizeof(uint8_t));
    memcpy(&(message.flags), &buf[1], sizeof(uint8_t));
    memcpy(&(message.glv_battery_voltage), &buf[2], sizeof(uint16_t));
    memcpy(&(message.temperature), &buf[4], sizeof(int16_t));
}

void RCU_status::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.state), sizeof(uint8_t));
    memcpy(&buf[1], &(message.flags), sizeof(uint8_t));
    memcpy(&buf[2], &(message.glv_battery_voltage), sizeof(uint16_t));
    memcpy(&buf[4], &(message.temperature), sizeof(int16_t));
}

uint8_t RCU_status::get_state() {
    return message.state;
}

uint8_t RCU_status::get_flags() {
    return message.flags;
}

bool RCU_status::get_bms_ok_high() {
    return message.flags & 0x1;
}

bool RCU_status::get_imd_okhs_high() {
    return (message.flags & 0x2) >> 1;
}

bool RCU_status::get_bms_imd_latched() {
    return (message.flags & 0x4) >> 2;
}

bool RCU_status::get_inverter_powered() {
    return (message.flags & 0x8) >> 3;
}

uint16_t RCU_status::get_glv_battery_voltage() {
    return message.glv_battery_voltage;
}

int16_t RCU_status::get_temperature() {
    return message.temperature;
}

void RCU_status::set_state(uint8_t state) {
    message.state = state;
}

void RCU_status::set_flags(uint8_t flags) {
    message.flags = flags;
}

void RCU_status::set_bms_ok_high(bool bms_ok_high) {
    message.flags = (message.flags & 0xFE) | (bms_ok_high & 0x1);
}

void RCU_status::set_imd_okhs_high(bool imd_okhs_high) {
    message.flags = (message.flags & 0xFD) | ((imd_okhs_high & 0x1) << 1);
}

void RCU_status::set_bms_imd_latched(bool bms_imd_latched) {
    message.flags = (message.flags & 0xFB) | ((bms_imd_latched & 0x1) << 2);
}

void RCU_status::set_inverter_powered(bool inverter_powered) {
    message.flags = (message.flags & 0xF7) | ((inverter_powered & 0x1) << 3);
}

void RCU_status::set_glv_battery_voltage(uint16_t glv_battery_voltage) {
    message.glv_battery_voltage = glv_battery_voltage;
}

void RCU_status::set_temperature(int16_t temperature) {
    message.temperature = temperature;
}
