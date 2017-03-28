/*
  BMS_currents.cpp - BMS current and charging state messaging handler
  Created by Shrivathsav Seshan, February 7, 2017.
 */

#include "HyTech17.h"

BMS_currents::BMS_currents() {
    bmsCurrentMessage = {};
}

BMS_currents::BMS_currents(uint8_t buf[]) {
    load(buf);
}

BMS_currents::BMS_currents(float _current, CHARGING_STATE state) {
    setCurrent(_current);
    setChargingState(state);
}

void BMS_currents::load(uint8_t buf[]) {
    bmsCurrentMessage = {};
    memcpy(&(bmsCurrentMessage.current), &buf[0], sizeof(float));
    int num;
    memcpy(&num, &buf[4], sizeof(int));
    if (num == 0) {
        bmsCurrentMessage.chargeState = DISCHARGING;
    } else if (num == 1) {
        bmsCurrentMessage.chargeState = CHARGING;
    } else if (num == 2) {
        bmsCurrentMessage.chargeState = UNKNOWN;
    }
}

void BMS_currents::write(uint8_t buf[]) {
    memcpy(&buf[0], &(bmsCurrentMessage.current), sizeof(float));
    memcpy(&buf[4], &(bmsCurrentMessage.chargeState), sizeof(int));
}

float BMS_currents::getCurrent() {
    return bmsCurrentMessage.current;
}

CHARGING_STATE BMS_currents::getChargingState() {
    return bmsCurrentMessage.chargeState;
}

void BMS_currents::setCurrent(float _current) {
    bmsCurrentMessage.current = _current;
}

void BMS_currents::setChargingState(CHARGING_STATE state) {
    bmsCurrentMessage.chargeState = state;
}
