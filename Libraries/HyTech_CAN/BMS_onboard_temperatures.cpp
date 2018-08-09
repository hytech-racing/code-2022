/*
 * BMS_onboard_temperatures.cpp - CAN message parser: Battery Management System PCB onboard temperatures message
 * Created by Yvone Yeh, February 8, 2018.
 */

#include "HyTech_CAN.h"

BMS_onboard_temperatures::BMS_onboard_temperatures() {
    message = {};
}

BMS_onboard_temperatures::BMS_onboard_temperatures(uint8_t buf[]) {
    load(buf);
}

BMS_onboard_temperatures::BMS_onboard_temperatures(int16_t average_temperature, int16_t low_temperature, int16_t high_temperature) {
    set_average_temperature(average_temperature);
    set_low_temperature(low_temperature);
    set_high_temperature(high_temperature);
}

void BMS_onboard_temperatures::load(uint8_t buf[]) {
    message = {};
    memcpy(&(message.average_temperature), &buf[0], sizeof(int16_t));
    memcpy(&(message.low_temperature), &buf[2], sizeof(int16_t));
    memcpy(&(message.high_temperature), &buf[4], sizeof(int16_t));
}

void BMS_onboard_temperatures::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.average_temperature), sizeof(int16_t));
    memcpy(&buf[2], &(message.low_temperature), sizeof(int16_t));
    memcpy(&buf[4], &(message.high_temperature), sizeof(int16_t));
}

int16_t BMS_onboard_temperatures::get_average_temperature() {
    return message.average_temperature;
}

int16_t BMS_onboard_temperatures::get_low_temperature() {
    return message.low_temperature;
}

int16_t BMS_onboard_temperatures::get_high_temperature() {
    return message.high_temperature;
}

void BMS_onboard_temperatures::set_average_temperature(int16_t average_temperature) {
    message.average_temperature = average_temperature;
}

void BMS_onboard_temperatures::set_low_temperature(int16_t low_temperature) {
    message.low_temperature = low_temperature;
}

void BMS_onboard_temperatures::set_high_temperature(int16_t high_temperature) {
    message.high_temperature = high_temperature;
}
