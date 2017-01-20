/*
  BMS_voltages.cpp - BMS voltage messaging handler
  Created by Shrivathsav Seshan, January 10, 2017.
 */

#include "HyTech17.h"

BMS_voltages::BMS_voltages() {
    bmsVoltages.avgVoltage = 0;
    bmsVoltages.lowVoltage = 0;
    bmsVoltages.highVoltage = 0;
}

/*
 * [b0 b1 b2 b3 b4 b5 b6 b7]
 * [avg avg low low high high - -]
 */
BMS_voltages::BMS_voltages(uint8_t buf[]) {
    load(buf);
}

BMS_voltages::BMS_voltages(uint16_t avg, uint16_t low, uint16_t high) {
    bmsVoltages.avgVoltage = avg;
    bmsVoltages.lowVoltage = low;
    bmsVoltages.highVoltage = high;
}

/*
 * Populate this object using the data stored in the specified byte array.
 */
void BMS_voltages::load(uint8_t buf[]) {
    memcpy(&(bmsVoltages.avgVoltage), buf, sizeof(uint16_t));
    memcpy(&(bmsVoltages.lowVoltage), buf + sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&(bmsVoltages.highVoltage), buf + sizeof(uint16_t) * 2, sizeof(uint16_t));
}

/*
 * Populates the specified byte array using the data stored in this object.
 */
void BMS_voltages::write(uint8_t buf[]) {
    memcpy(buf[0], &bmsVoltages.avgVoltage, sizeof(uint16_t));
    memcpy(buf[0 + sizeof(uint16_t)], &bmsVoltages.avgVoltage, sizeof(uint16_t));
    memcpy(buf[0 + sizeof(uint16_t) * 2], &bmsVoltages.avgVoltage, sizeof(uint16_t));
}

uint16_t BMS_voltages::getAverage() {
    return bmsVoltages.avgVoltage;
}

uint16_t BMS_voltages::getLow() {
    return bmsVoltages.lowVoltage;
}

uint16_t BMS_voltages::getHigh() {
    return bmsVoltages.highVoltage;
}

void BMS_voltages::setAverage(uint16_t avg) {
    bmsVoltages.avgVoltage = avg;
}

void BMS_voltages::setLow(uint16_t low) {
    bmsVoltages.lowVoltage = low;
}

void BMS_voltages::setHigh(uint16_t high) {
    bmsVoltages.highVoltage = high;
}
