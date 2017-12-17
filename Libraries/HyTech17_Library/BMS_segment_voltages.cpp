/*
  BMS_segment_voltages.cpp - BMS detailed voltage message
  Created by Nathan Cheek, December 17, 2017.
 */

#include "HyTech17.h"

BMS_segment_voltages::BMS_segment_voltages() {
    message = {};
}

BMS_segment_voltages::BMS_segment_voltages(uint8_t buf[]) {
    load(buf);
}

BMS_segment_voltages::BMS_segment_voltages(uint8_t segment_id, uint16_t voltages_set_1, uint16_t voltages_set_2, uint16_t voltages_set_3) {
    message = {};
    message.segment_id = segment_id;
    message.voltages_set_1 = voltages_set_1;
    message.voltages_set_2 = voltages_set_2;
    message.voltages_set_3 = voltages_set_3;
}

void BMS_segment_voltages::load(uint8_t buf[]) {
    memcpy(&(message.segment_id), &buf[0], sizeof(uint8_t));
    memcpy(&(message.voltages_set_1), &buf[1], sizeof(uint16_t));
    memcpy(&(message.voltages_set_2), &buf[3], sizeof(uint16_t));
    memcpy(&(message.voltages_set_3), &buf[5], sizeof(uint16_t));
}

void BMS_segment_voltages::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.segment_id), sizeof(uint8_t));
    memcpy(&buf[1], &(message.voltages_set_1), sizeof(uint16_t));
    memcpy(&buf[3], &(message.voltages_set_2), sizeof(uint16_t));
    memcpy(&buf[5], &(message.voltages_set_3), sizeof(uint16_t));
}

uint8_t BMS_segment_voltages::get_segment_id() {
    return message.segment_id;
}

uint16_t BMS_segment_voltages::get_voltages_set_1() {
    return message.voltages_set_1;
}

uint16_t BMS_segment_voltages::get_voltages_set_2() {
    return message.voltages_set_2;
}

uint16_t BMS_segment_voltages::get_voltages_set_3() {
    return message.voltages_set_3;
}

void BMS_segment_voltages::set_segment_id(uint8_t segment_id) {
    message.segment_id = segment_id;
}

void BMS_segment_voltages::set_voltages_set_1(uint16_t voltages_set_1) {
    message.voltages_set_1 = voltages_set_1;
}

void BMS_segment_voltages::set_voltages_set_2(uint16_t voltages_set_2) {
    message.voltages_set_2 = voltages_set_2;
}

void BMS_segment_voltages::set_voltages_set_3(uint16_t voltages_set_3) {
    message.voltages_set_3 = voltages_set_3;
}