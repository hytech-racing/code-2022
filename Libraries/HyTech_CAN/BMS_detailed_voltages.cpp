/*
 * BMS_detailed_voltages.cpp - CAN message parser: Battery Management System detailed voltages message
 * Created by Nathan Cheek, December 20, 2017.
 */

#include "HyTech_CAN.h"

BMS_detailed_voltages::BMS_detailed_voltages() {
    message = {};
}

BMS_detailed_voltages::BMS_detailed_voltages(uint8_t buf[]) {
    load(buf);
}

BMS_detailed_voltages::BMS_detailed_voltages(uint8_t ic_id, uint8_t group_id, uint16_t voltage_0, uint16_t voltage_1, uint16_t voltage_2) {
    message = {};
    set_ic_id(ic_id);
    set_group_id(group_id);
    set_voltage_0(voltage_0);
    set_voltage_1(voltage_1);
    set_voltage_2(voltage_2);
}

void BMS_detailed_voltages::load(uint8_t buf[]) {
    memcpy(&(message.ic_id_group_id), &buf[0], sizeof(uint8_t));
    memcpy(&(message.voltage_0), &buf[1], sizeof(uint16_t));
    memcpy(&(message.voltage_1), &buf[3], sizeof(uint16_t));
    memcpy(&(message.voltage_2), &buf[5], sizeof(uint16_t));
}

void BMS_detailed_voltages::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.ic_id_group_id), sizeof(uint8_t));
    memcpy(&buf[1], &(message.voltage_0), sizeof(uint16_t));
    memcpy(&buf[3], &(message.voltage_1), sizeof(uint16_t));
    memcpy(&buf[5], &(message.voltage_2), sizeof(uint16_t));
}

uint8_t BMS_detailed_voltages::get_ic_id() {
    return message.ic_id_group_id & 0x0F;
}

uint8_t BMS_detailed_voltages::get_group_id() {
    return (message.ic_id_group_id & 0xF0) >> 4;
}

uint16_t BMS_detailed_voltages::get_voltage_0() {
    return message.voltage_0;
}

uint16_t BMS_detailed_voltages::get_voltage_1() {
    return message.voltage_1;
}

uint16_t BMS_detailed_voltages::get_voltage_2() {
    return message.voltage_2;
}

uint16_t BMS_detailed_voltages::get_voltage(uint8_t voltage_id) {
    if (voltage_id == 0) {
        return message.voltage_0;
    } else if (voltage_id == 1) {
        return message.voltage_1;
    } else if (voltage_id == 2) {
        return message.voltage_2;
    }
    return 0;
}

void BMS_detailed_voltages::set_ic_id(uint8_t ic_id) {
    message.ic_id_group_id = (message.ic_id_group_id & 0xF0) | (ic_id & 0xF);
}

void BMS_detailed_voltages::set_group_id(uint8_t group_id) {
    message.ic_id_group_id = (message.ic_id_group_id & 0x0F) | ((group_id & 0xF) << 4);
}

void BMS_detailed_voltages::set_voltage_0(uint16_t voltage_0) {
    message.voltage_0 = voltage_0;
}

void BMS_detailed_voltages::set_voltage_1(uint16_t voltage_1) {
    message.voltage_1 = voltage_1;
}

void BMS_detailed_voltages::set_voltage_2(uint16_t voltage_2) {
    message.voltage_2 = voltage_2;
}

void BMS_detailed_voltages::set_voltage(uint8_t voltage_id, uint16_t voltage) {
    if (voltage_id == 0) {
        message.voltage_0 = voltage;
    } else if (voltage_id == 1) {
        message.voltage_1 = voltage;
    } else if (voltage_id == 2) {
        message.voltage_2 = voltage;
    }
}