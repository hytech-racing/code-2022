/*
 *BMS_coulomb_counts - CAN message parser: Battery Management System coulomb count message
 * Created by Shaan Dhawan, March 26, 2019.
 */

#include "HyTech_CAN.h"

BMS_coulomb_counts::BMS_coulomb_counts() {
    message = {};
}

BMS_coulomb_counts::BMS_coulomb_counts(uint8_t buf[]) {
    load(buf);
}

BMS_coulomb_counts::BMS_coulomb_counts(uint32_t total_charge, uint32_t total_discharge) {
    set_total_charge(total_charge);
    set_total_discharge(total_discharge);
}

void BMS_coulomb_counts::load(uint8_t buf[]) {
    message = {};
    memcpy(&(message.total_charge), &buf[0], sizeof(uint32_t));
    memcpy(&(message.total_discharge), &buf[4], sizeof(uint32_t));
}

void BMS_coulomb_counts::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.total_charge), sizeof(uint32_t));
    memcpy(&buf[4], &(message.total_discharge), sizeof(uint32_t));
}

uint32_t BMS_coulomb_counts::get_total_charge() {
    return message.total_charge;
}

uint32_t BMS_coulomb_counts::get_total_discharge() {
    return message.total_discharge;
}


void BMS_coulomb_counts::set_total_charge(uint32_t total_charge) {
    message.total_charge = total_charge;
}

void BMS_coulomb_counts::set_total_discharge(uint32_t total_discharge) {
    message.total_discharge = total_discharge;
}

