/*
  BMS_voltages.cpp - BMS voltage messaging handler
  Created by Shrivathsav Seshan, January 10, 2017.
 */

#include "HyTech17.h"

BMS_voltages::BMS_voltages() {
    bmsVoltageMessage = {};
}

/*
 * [b0 b1 b2 b3 b4 b5 b6 b7]
 * [avg avg low low high high - -]
 */
BMS_voltages::BMS_voltages(uint8_t buf[]) {
    load(buf);
}

BMS_voltages::BMS_voltages(uint16_t avg, uint16_t low, uint16_t high) {
    bmsVoltageMessage = {};
    bmsVoltageMessage.avgVoltage = avg;
    bmsVoltageMessage.lowVoltage = low;
    bmsVoltageMessage.highVoltage = high;
}

/*
 * Populate this object using the data stored in the specified byte array.
 */
void BMS_voltages::load(uint8_t buf[]) {
    memcpy(&(bmsVoltageMessage.avgVoltage), buf, sizeof(uint16_t));
    memcpy(&(bmsVoltageMessage.lowVoltage), buf + sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&(bmsVoltageMessage.highVoltage), buf + sizeof(uint16_t * 2), sizeof(uint16_t));
}

/*
 * Populates the specified byte array using the data stored in this object.
 */
void BMS_voltages::write(uint8_t buf[]) {
    memcpy(buf[0], &bmsVoltageMessage.avgVoltage, sizeof(uint16_t));
    memcpy(buf[0 + sizeof(uint16_t)], &bmsVoltageMessage.avgVoltage, sizeof(uint16_t));
    memcpy(buf[0 + sizeof(uint16_t) * 2], &bmsVoltageMessage.avgVoltage, sizeof(uint16_t));
}

uint16_t BMS_voltages::getAverage() {
    return bmsVoltageMessage.avgVoltage;
}

uint16_t BMS_voltages::getLow() {
    return bmsVoltageMessage.lowVoltage;
}

uint16_t BMS_voltages::getHigh() {
    return bmsVoltageMessage.highVoltage;
}

void BMS_voltages::setAverage(uint16_t avg) {
    bmsVoltageMessage.avgVoltage = avg;
}

void BMS_voltages::setLow(uint16_t low) {
    bmsVoltageMessage.lowVoltage = low;
}

void BMS_voltages::setHigh(uint16_t high) {
    bmsVoltageMessage.highVoltage = high;
}
