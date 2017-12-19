/*
  BMS_status.cpp - BMS status messaging handler
  Created by Shrivathsav Seshan, Feb 16, 2017.
 */

#include "HyTech17.h"

BMS_status::BMS_status() {
    message = {};
}

BMS_status::BMS_status(uint8_t buf[]) {
    load(buf);
}

void BMS_status::load(uint8_t buf[]) {
    memcpy(&(message.state), &buf[0], sizeof(uint8_t));
    memcpy(&(message.error_flags), &buf[1], sizeof(uint8_t));
    memcpy(&(message.current), &buf[2], sizeof(int16_t));
}

void BMS_status::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.state), sizeof(uint8_t));
    memcpy(&buf[1], &(message.error_flags), sizeof(uint8_t));
    memcpy(&buf[2], &(message.current), sizeof(int16_t));
}

uint8_t BMS_status::get_state() {
    return message.state;
}

uint8_t BMS_status::get_error_flags() {
    return message.error_flags;
}

bool BMS_status::get_overvoltage() {
    return message.error_flags & 0x01;
}

bool BMS_status::get_undervoltage() {
    return (message.error_flags & 0x02) >> 1;
}

bool BMS_status::get_total_voltage_high() {
    return (message.error_flags & 0x04) >> 2;
}

bool BMS_status::get_discharge_overcurrent() {
    return (message.error_flags & 0x08) >> 3;
}

bool BMS_status::get_charge_overcurrent() {
    return (message.error_flags & 0x10) >> 4;
}

bool BMS_status::get_discharge_overtemp() {
    return (message.error_flags & 0x20) >> 5;
}

bool BMS_status::get_charge_overtemp() {
    return (message.error_flags & 0x40) >> 6;
}

int16_t BMS_status::get_current() {
    return message.current;
}

void BMS_status::set_state(uint8_t state) {
    message.state = state;
}

void BMS_status::set_error_flags(uint8_t error_flags) {
    message.error_flags = error_flags;
}

void BMS_status::set_overvoltage(bool overvoltage) {
    message.error_flags = (message.error_flags & 0xFE) | (overvoltage & 0x01);
}

void BMS_status::set_undervoltage(bool undervoltage) {
    message.error_flags = (message.error_flags & 0xFD) | (undervoltage & 0x02);
}

void BMS_status::set_total_voltage_high(bool total_voltage_high) {
    message.error_flags = (message.error_flags & 0xFB) | (total_voltage_high & 0x04);
}

void BMS_status::set_discharge_overcurrent(bool discharge_overcurrent) {
    message.error_flags = (message.error_flags & 0xF7) | (discharge_overcurrent & 0x08);
}

void BMS_status::set_charge_overcurrent(bool charge_overcurrent) {
    message.error_flags = (message.error_flags & 0xEF) | (charge_overcurrent & 0x10);
}

void BMS_status::set_discharge_overtemp(bool discharge_overtemp) {
    message.error_flags = (message.error_flags & 0xDF) | (discharge_overtemp & 0x20);
}

void BMS_status::set_charge_overtemp(bool charge_overtemp) {
    message.error_flags = (message.error_flags & 0xBF) | (charge_overtemp & 0x40);
}

void BMS_status::set_current(int16_t current) {
    message.current = current;
}