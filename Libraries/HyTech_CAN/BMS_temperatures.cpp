/*
 * BMS_temperatures.cpp - CAN message parser: Battery Management System temperatures message
 * Created by Shrivathsav Seshan, February 7, 2017.
 */

#include "HyTech_CAN.h"

BMS_temperatures::BMS_temperatures() {
    message = {};
}

BMS_temperatures::BMS_temperatures(uint8_t buf[]) {
    load(buf);
}

BMS_temperatures::BMS_temperatures(int16_t average_temperature, int16_t low_temperature, int16_t high_temperature) {
    set_average_temperature(average_temperature);
    set_low_temperature(low_temperature);
    set_high_temperature(high_temperature);
}

void BMS_temperatures::load(uint8_t buf[]) {
    message = {};
    memcpy(&(message.average_temperature), &buf[0], sizeof(int16_t));
    memcpy(&(message.low_temperature), &buf[2], sizeof(int16_t));
    memcpy(&(message.high_temperature), &buf[4], sizeof(int16_t));
}

void BMS_temperatures::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.average_temperature), sizeof(int16_t));
    memcpy(&buf[2], &(message.low_temperature), sizeof(int16_t));
    memcpy(&buf[4], &(message.high_temperature), sizeof(int16_t));
}

int16_t BMS_temperatures::get_average_temperature() {
    return message.average_temperature;
}

int16_t BMS_temperatures::get_low_temperature() {
    return message.low_temperature;
}

int16_t BMS_temperatures::get_high_temperature() {
    return message.high_temperature;
}

void BMS_temperatures::set_average_temperature(int16_t average_temperature) {
    message.average_temperature = average_temperature;
}

void BMS_temperatures::set_low_temperature(int16_t low_temperature) {
    message.low_temperature = low_temperature;
}

void BMS_temperatures::set_high_temperature(int16_t high_temperature) {
    message.high_temperature = high_temperature;
}
