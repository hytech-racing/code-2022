/*
 * BMS_onboard_detailed_temperatures.cpp - CAN message parser: Battery Management System PCB onboard detailed temperatures message
 * Created by Yvonne Yeh, February 8, 2018.
 */

#include "HyTech_CAN.h"

BMS_onboard_detailed_temperatures::BMS_onboard_detailed_temperatures() {
    message = {};
}

BMS_onboard_detailed_temperatures::BMS_onboard_detailed_temperatures(uint8_t buf[]) {
    load(buf);
}

BMS_onboard_detailed_temperatures::BMS_onboard_detailed_temperatures(uint8_t ic_id, int16_t temperature_0, int16_t temperature_1) {
    message = {};
    set_ic_id(ic_id);
    set_temperature_0(temperature_0);
    set_temperature_1(temperature_1);
}

void BMS_onboard_detailed_temperatures::load(uint8_t buf[]) {
    memcpy(&(message.ic_id), &buf[0], sizeof(uint8_t));
    memcpy(&(message.temperature_0), &buf[1], sizeof(int16_t));
    memcpy(&(message.temperature_1), &buf[3], sizeof(int16_t));
}

void BMS_onboard_detailed_temperatures::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.ic_id), sizeof(uint8_t));
    memcpy(&buf[1], &(message.temperature_0), sizeof(int16_t));
    memcpy(&buf[3], &(message.temperature_1), sizeof(int16_t));
}

uint8_t BMS_onboard_detailed_temperatures::get_ic_id() {
    return message.ic_id;
}

int16_t BMS_onboard_detailed_temperatures::get_temperature_0() {
    return message.temperature_0;
}

int16_t BMS_onboard_detailed_temperatures::get_temperature_1() {
    return message.temperature_1;
}

int16_t BMS_onboard_detailed_temperatures::get_temperature(uint8_t temperature_id) {
    if (temperature_id == 0) {
        return message.temperature_0;
    } else if (temperature_id == 1) {
        return message.temperature_1;
    } 
    return 0;
}

void BMS_onboard_detailed_temperatures::set_ic_id(uint8_t ic_id) {
    message.ic_id = ic_id;
}

void BMS_onboard_detailed_temperatures::set_temperature_0(int16_t temperature_0) {
    message.temperature_0 = temperature_0;
}

void BMS_onboard_detailed_temperatures::set_temperature_1(int16_t temperature_1) {
    message.temperature_1 = temperature_1;
}

void BMS_onboard_detailed_temperatures::set_temperature(uint8_t temperature_id, int16_t temperature) {
    if (temperature_id == 0) {
        message.temperature_0 = temperature;
    } else if (temperature_id == 1) {
        message.temperature_1 = temperature;
    }
}