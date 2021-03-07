/*
 * BMS_balancing_status.cpp - CAN message parser: Battery Management System balancing status message
 * Created by Nathan Cheek, March 25, 2019.
 */

#include "HyTech_CAN.h"

/*
 * 5-byte message structure from MSB to LSB
 * [9 bits: IC 4 balancing data] [9 bits: IC 3 balancing data] [9 bits: IC 2 balancing data] [9 bits: IC 1 balancing data] [3 bits: empty] [1 bit: group]
 * 
 * Structure of each IC section from MSB to LSB
 * [Cell 9 balancing] [Cell 8 balancing] [Cell 7 balancing] [Cell 6 balancing] [Cell 5 balancing] [Cell 4 balancing] [Cell 3 balancing] [Cell 2 balancing] [Cell 1 balancing]
 */

// Make sure to cast things to uint64_t before doing large bit-shifts (>32 bits)


BMS_balancing_status::BMS_balancing_status() {
    message = {};
}

BMS_balancing_status::BMS_balancing_status(uint8_t buf[]) {
    load(buf);
}

BMS_balancing_status::BMS_balancing_status(uint8_t group_id, int64_t balancing) {
    message = {};
    set_group_id(group_id);
    set_balancing(balancing);
}

void BMS_balancing_status::load(uint8_t buf[]) {
    memcpy(&(message), &buf[0], sizeof(CAN_message_bms_balancing_status_t));
}

void BMS_balancing_status::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message), sizeof(CAN_message_bms_balancing_status_t));
}

uint8_t BMS_balancing_status::get_group_id() {
    return message & 0xF;
}

uint64_t BMS_balancing_status::get_balancing() {
    return (message >> 0x4) & 0xFFFFFFFFF;
}

uint16_t BMS_balancing_status::get_ic_balancing(uint8_t ic_id) {
    return (message >> (0x4 + 0x9 * ic_id)) & 0x1FF;
}

bool BMS_balancing_status::get_cell_balancing(uint8_t ic_id, uint16_t cell_id) {
    return (get_ic_balancing(ic_id) >> cell_id) & 0x1;
}

void BMS_balancing_status::set_group_id(uint8_t group_id) {
    message = (message & 0xFFFFFFFFF0) | (group_id & 0xF);
}

void BMS_balancing_status::set_balancing(uint64_t balancing) {
    message = (message & 0x4) | ((balancing & 0xFFFFFFFFF) << 0x4);
}

void BMS_balancing_status::set_ic_balancing(uint8_t ic_id, uint16_t balancing) {
    message = (message & ~(((uint64_t) 0x1FF) << (0x4 + 0x9 * ic_id))) | (((uint64_t) (balancing & 0x1FF)) << (0x4 + 0x9 * ic_id));
}

void BMS_balancing_status::set_cell_balancing(uint8_t ic_id, uint8_t cell_id, bool balancing) {
    message = (message & ~(((uint64_t) 0x1) << (0x4 + 0x9 * ic_id + cell_id))) | (((uint64_t) (balancing & 0x1)) << (0x4 + 0x9 * ic_id + cell_id));
}
