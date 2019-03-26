/*
 * BMS_balancing_status.cpp - CAN message parser: Battery Management System balancing status message
 * Created by Nathan Cheek, March 25, 2019.
 */

#include "HyTech_CAN.h"

BMS_balancing_status::BMS_balancing_status() {
    message = {};
}

BMS_balancing_status::BMS_balancing_status(uint8_t buf[]) {
    load(buf);
}

BMS_balancing_status::BMS_balancing_status(uint8_t group_id, int64_t balancing_status) {
    message = {};
    set_group_id(group_id);
    set_balancing_status(balancing_status);
}

void BMS_balancing_status::load(uint8_t buf[]) {
    memcpy(&(message), &buf[0], sizeof(CAN_message_bms_balancing_status_t));
}

void BMS_balancing_status::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message), sizeof(CAN_message_bms_balancing_status_t));
}

uint8_t BMS_balancing_status::get_group_id() {
    return message & 0x1;
}

uint64_t BMS_balancing_status::get_balancing_status() {
    return (message >> 0x4) & 0xFFFFFFFFF;
}

uint32_t BMS_balancing_status::get_segment_balancing_status(uint8_t segment_id) {
    return (message >> (0x4 + 0x12 * segment_id)) & 0x3FFFF;
}

bool BMS_balancing_status::get_cell_balancing_status(uint8_t segment_id, uint16_t cell_id) {
    return (get_segment_balancing_status(segment_id) >> cell_id) & 0x1;
}

void BMS_balancing_status::set_group_id(uint8_t group_id) {
    message = (message & 0xFFFFFFFFFE) | (group_id & 0x1);
}

void BMS_balancing_status::set_balancing_status(uint64_t balancing_status) {
    message = (message & 0x4) | ((balancing_status & 0xFFFFFFFFF) << 0x4);
}

void BMS_balancing_status::set_segment_balancing_status(uint8_t segment_id, uint32_t balancing_status) {
    message = (message & ~(0x3FFFF << (0x4 + 0x12 * segment_id))) | ((balancing_status & 0x3FFFF) << (0x4 + 0x12 * segment_id));
}

void BMS_balancing_status::set_cell_balancing_status(uint8_t segment_id, uint8_t cell_id, bool balancing_status) {
    message = (message & ~(0x1 << (0x4 + 0x12 * segment_id + cell_id))) | ((balancing_status & 0x1) << (0x4 + 0x12 * segment_id + cell_id));
}
