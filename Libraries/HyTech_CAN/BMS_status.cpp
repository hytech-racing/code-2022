/*
 * BMS_status.cpp - CAN message parser: Battery Management System status message
 * Created by Shrivathsav Seshan, Feb 16, 2017.
 */

#include "HyTech_CAN.h"

BMS_status::BMS_status() {
    message = {};
}

BMS_status::BMS_status(uint8_t buf[]) {
    load(buf);
}

void BMS_status::load(uint8_t buf[]) {
    memcpy(&(message.state), &buf[0], sizeof(uint8_t));
    memcpy(&(message.error_flags), &buf[1], sizeof(uint16_t));
    memcpy(&(message.current), &buf[3], sizeof(int16_t));
    memcpy(&(message.flags), &buf[5], sizeof(uint8_t));
}

void BMS_status::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.state), sizeof(uint8_t));
    memcpy(&buf[1], &(message.error_flags), sizeof(uint16_t));
    memcpy(&buf[3], &(message.current), sizeof(int16_t));
    memcpy(&buf[5], &(message.flags), sizeof(uint8_t));
}

uint8_t BMS_status::get_state() {
    return message.state;
}

uint16_t BMS_status::get_error_flags() {
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

bool BMS_status::get_undertemp() {
    return (message.error_flags & 0x80) >> 7;
}

bool BMS_status::get_onboard_overtemp() {
    return (message.error_flags & 0x100) >> 8;
}

int16_t BMS_status::get_current() {
    return message.current;
}

uint8_t BMS_status::get_flags() {
    return message.flags;
}

bool BMS_status::get_shutdown_g_above_threshold() {
    return message.flags & 0x1;
}

bool BMS_status::get_shutdown_h_above_threshold() {
    return (message.flags & 0x02) >> 1;
}

void BMS_status::set_state(uint8_t state) {
    message.state = state;
}

void BMS_status::set_error_flags(uint16_t error_flags) {
    message.error_flags = error_flags;
}

void BMS_status::set_overvoltage(bool overvoltage) {
    message.error_flags = (message.error_flags & 0xFFFE) | (overvoltage & 0x1);
}

void BMS_status::set_undervoltage(bool undervoltage) {
    message.error_flags = (message.error_flags & 0xFFFD) | ((undervoltage & 0x1) << 1);
}

void BMS_status::set_total_voltage_high(bool total_voltage_high) {
    message.error_flags = (message.error_flags & 0xFFFB) | ((total_voltage_high & 0x1) << 2);
}

void BMS_status::set_discharge_overcurrent(bool discharge_overcurrent) {
    message.error_flags = (message.error_flags & 0xFFF7) | ((discharge_overcurrent & 0x1) << 3);
}

void BMS_status::set_charge_overcurrent(bool charge_overcurrent) {
    message.error_flags = (message.error_flags & 0xFFEF) | ((charge_overcurrent & 0x1) << 4);
}

void BMS_status::set_discharge_overtemp(bool discharge_overtemp) {
    message.error_flags = (message.error_flags & 0xFFDF) | ((discharge_overtemp & 0x1) << 5);
}

void BMS_status::set_charge_overtemp(bool charge_overtemp) {
    message.error_flags = (message.error_flags & 0xFFBF) | ((charge_overtemp & 0x1) << 6);
}

void BMS_status::set_undertemp(bool undertemp) {
    message.error_flags = (message.error_flags & 0xFF7F) | ((undertemp & 0x1) << 7);
}

void BMS_status::set_onboard_overtemp(bool onboard_overtemp) {
    message.error_flags = (message.error_flags & 0xFEFF) | ((onboard_overtemp & 0x1) << 8);
}

void BMS_status::set_current(int16_t current) {
    message.current = current;
}

void BMS_status::set_flags(uint8_t flags) {
    message.flags = flags;
}

void BMS_status::set_shutdown_g_above_threshold(bool shutdown_g_above_threshold) {
    message.flags = (message.flags & 0xFFFE) | (shutdown_g_above_threshold & 0x1);
}

void BMS_status::set_shutdown_h_above_threshold(bool shutdown_h_above_threshold) {
    message.flags = (message.flags & 0xFFFD) | ((shutdown_h_above_threshold & 0x1) << 1);
}