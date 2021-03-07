/*
 * BMS_voltages.cpp - CAN message parser: Battery Management System voltages message
 * Created by Shrivathsav Seshan, January 10, 2017.
 */

#include "HyTech_CAN.h"

BMS_voltages::BMS_voltages() {
    message = {};
}

BMS_voltages::BMS_voltages(uint8_t buf[]) {
    load(buf);
}

BMS_voltages::BMS_voltages(uint16_t average_voltage, uint16_t low_voltage, uint16_t high_voltage, uint16_t total_voltage) {
    message = {};
    message.average_voltage = average_voltage;
    message.low_voltage = low_voltage;
    message.high_voltage = high_voltage;
    message.total_voltage = total_voltage;
}

/*
 * Populate this object using the data stored in the specified byte array.
 */
void BMS_voltages::load(uint8_t buf[]) {
    memcpy(&(message.average_voltage), &buf[0], sizeof(uint16_t));
    memcpy(&(message.low_voltage), &buf[2], sizeof(uint16_t));
    memcpy(&(message.high_voltage), &buf[4], sizeof(uint16_t));
    memcpy(&(message.total_voltage), &buf[6], sizeof(uint16_t));
}

/*
 * Populates the specified byte array using the data stored in this object.
 */
void BMS_voltages::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.average_voltage), sizeof(uint16_t));
    memcpy(&buf[2], &(message.low_voltage), sizeof(uint16_t));
    memcpy(&buf[4], &(message.high_voltage), sizeof(uint16_t));
    memcpy(&buf[6], &(message.total_voltage), sizeof(uint16_t));
}

uint16_t BMS_voltages::get_average() {
    return message.average_voltage;
}

uint16_t BMS_voltages::get_low() {
    return message.low_voltage;
}

uint16_t BMS_voltages::get_high() {
    return message.high_voltage;
}

uint16_t BMS_voltages::get_total() {
    return message.total_voltage;
}

void BMS_voltages::set_average(uint16_t average_voltage) {
    message.average_voltage = average_voltage;
}

void BMS_voltages::set_low(uint16_t low_voltage) {
    message.low_voltage = low_voltage;
}

void BMS_voltages::set_high(uint16_t high_voltage) {
    message.high_voltage = high_voltage;
}

void BMS_voltages::set_total(uint16_t total_voltage) {
    message.total_voltage = total_voltage;
}
