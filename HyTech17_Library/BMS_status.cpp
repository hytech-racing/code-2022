/*
  BMS_status.cpp - BMS status messaging handler
  Created by Shrivathsav Seshan, Feb 16, 2017.
 */

#include "HyTech17.h"

BMS_status::BMS_status() {
    bmsErrorMessage = {};
}

BMS_status::BMS_status(uint8_t buf[]) {
    load(buf);
}

void BMS_status::load(uint8_t buf[]) {
    memcpy(&(bmsErrorMessage.errorFlagsByte1), &buf[0], sizeof(uint8_t));
    memcpy(&(bmsErrorMessage.errorFlagsByte2), &buf[1], sizeof(uint8_t));
    memcpy(&(bmsErrorMessage.BMSStatusOK), &buf[2], sizeof(uint8_t));
}

void BMS_status::write(uint8_t buf[]) {
    memcpy(&buf[0], &(bmsErrorMessage.errorFlagsByte1), sizeof(uint8_t));
    memcpy(&buf[1], &(bmsErrorMessage.errorFlagsByte2), sizeof(uint8_t));
    memcpy(&buf[2], &(bmsErrorMessage.BMSStatusOK), sizeof(uint8_t));
}

/***************GETTERS*****************/
bool BMS_status::getDischargeOvervoltage() {
    uint8_t val = bmsErrorMessage.errorFlagsByte1 & 0b10000000;
    return val > 0;
}

bool BMS_status::getDischargeUndervoltage() {
    uint8_t val = bmsErrorMessage.errorFlagsByte1 & 0b01000000;
    return val > 0;
}

bool BMS_status::getChargeOvervoltage() {
    uint8_t val = bmsErrorMessage.errorFlagsByte1 & 0b00100000;
    return val > 0;
}

bool BMS_status::getChargeUndervoltage() {
    uint8_t val = bmsErrorMessage.errorFlagsByte1 & 0b00010000;
    return val > 0;
}

bool BMS_status::getDischargeOvercurrent() {
    uint8_t val = bmsErrorMessage.errorFlagsByte1 & 0b00001000;
    return val > 0;
}

bool BMS_status::getDischargeUndercurrent() {
    uint8_t val = bmsErrorMessage.errorFlagsByte1 & 0b00000100;
    return val > 0;
}

bool BMS_status::getChargeOvercurrent() {
    uint8_t val = bmsErrorMessage.errorFlagsByte1 & 0b00000010;
    return val > 0;
}

bool BMS_status::getChargeUndercurrent() {
    uint8_t val = bmsErrorMessage.errorFlagsByte1 & 0b00000001;
    return val > 0;
}

bool BMS_status::getDischargeOvertemp() {
    uint8_t val = bmsErrorMessage.errorFlagsByte2 & 0b10000000;
    return val > 0;
}

bool BMS_status::getDischargeUndertemp() {
    uint8_t val = bmsErrorMessage.errorFlagsByte2 & 0b01000000;
    return val > 0;
}

bool BMS_status::getChargeOvertemp() {
    uint8_t val = bmsErrorMessage.errorFlagsByte2 & 0b00100000;
    return val > 0;
}

bool BMS_status::getChargeUndertemp() {
    uint8_t val = bmsErrorMessage.errorFlagsByte2 & 0b00010000;
    return val > 0;
}

bool BMS_status::getBMSStatusOK() {
    return bmsErrorMessage.BMSStatusOK > 0;
}

/***************SETTERS*****************/
void BMS_status::setDischargeOvervoltage(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b10000000;
    }
    bmsErrorMessage.errorFlagsByte1 = bmsErrorMessage.errorFlagsByte1 | val;
}

void BMS_status::setDischargeUndervoltage(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b01000000;
    }
    bmsErrorMessage.errorFlagsByte1 = bmsErrorMessage.errorFlagsByte1 | val;
}

void BMS_status::setChargeOvervoltage(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b00100000;
    }
    bmsErrorMessage.errorFlagsByte1 = bmsErrorMessage.errorFlagsByte1 | val;
}

void BMS_status::setChargeUndervoltage(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b00010000;
    }
    bmsErrorMessage.errorFlagsByte1 = bmsErrorMessage.errorFlagsByte1 | val;
}

void BMS_status::setDischargeOvercurrent(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b00001000;
    }
    bmsErrorMessage.errorFlagsByte1 = bmsErrorMessage.errorFlagsByte1 | val;
}

void BMS_status::setDischargeUndercurrent(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b00000100;
    }
    bmsErrorMessage.errorFlagsByte1 = bmsErrorMessage.errorFlagsByte1 | val;
}

void BMS_status::setChargeOvercurrent(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b00000010;
    }
    bmsErrorMessage.errorFlagsByte1 = bmsErrorMessage.errorFlagsByte1 | val;
}

void BMS_status::setChargeUndercurrent(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b00000001;
    }
    bmsErrorMessage.errorFlagsByte1 = bmsErrorMessage.errorFlagsByte1 | val;
}

void BMS_status::setDischargeOvertemp(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b10000000;
    }
    bmsErrorMessage.errorFlagsByte2 = bmsErrorMessage.errorFlagsByte2 | val;
}

void BMS_status::setDischargeUndertemp(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b01000000;
    }
    bmsErrorMessage.errorFlagsByte2 = bmsErrorMessage.errorFlagsByte2 | val;
}

void BMS_status::setChargeOvertemp(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b00100000;
    }
    bmsErrorMessage.errorFlagsByte2 = bmsErrorMessage.errorFlagsByte2 | val;
}

void BMS_status::setChargeUndertemp(bool flag) {
    uint8_t val = 0;
    if (flag) {
        val = 0b00010000;
    }
    bmsErrorMessage.errorFlagsByte2 = bmsErrorMessage.errorFlagsByte2 | val;
}

void BMS_status::setBMSStatusOK(bool flag) {
    if (flag) bmsErrorMessage.BMSStatusOK = 1;
    else bmsErrorMessage.BMSStatusOK = 0;
}
