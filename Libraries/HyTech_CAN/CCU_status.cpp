/*
 * CCU_status.cpp - CAN message parser: Charger Control Unit status message
 * Created by Nathan Cheek, December 17, 2017.
 */

#include "HyTech_CAN.h"

CCU_status::CCU_status() {
    message = {};
}

CCU_status::CCU_status(uint8_t buf[8]) {
    load(buf);
}

void CCU_status::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.charger_enabled), &buf[0], sizeof(uint8_t));
}

void CCU_status::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.charger_enabled), sizeof(uint8_t));
}

bool CCU_status::get_charger_enabled() {
    return message.charger_enabled & 0x1;
}

void CCU_status::set_charger_enabled(bool charger_enabled) {
    message.charger_enabled = charger_enabled & 0x1;
}
