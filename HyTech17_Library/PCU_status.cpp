/*
  PCU_status.cpp - HyTech Power Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech17.h"

PCU_status::PCU_status() {
    message = {};
}

PCU_status::PCU_status(uint8_t buf[8]) {
  load(buf);
}

PCU_status::PCU_status(uint8_t state, bool bms_fault, bool imd_fault, bool okhs_check, bool discharge_check) {
  set_state(state);
  set_bms_fault(bms_fault);
  set_imd_fault(imd_fault);
  set_okhs_check(okhs_check);
  set_discharge_check(discharge_check);
}

void PCU_status::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.state), &buf[0], sizeof(uint8_t));
    uint8_t booleanByte = 0;
    memcpy(&booleanByte, &buf[1], sizeof(uint8_t));
    message.bms_fault = booleanByte > 0;
    memcpy(&booleanByte, &buf[2], sizeof(uint8_t));
    message.imd_fault = booleanByte > 0;
    memcpy(&booleanByte, &buf[3], sizeof(uint8_t));
    message.okhs_check = booleanByte > 0;
    memcpy(&booleanByte, &buf[4], sizeof(uint8_t));
    message.discharge_check = booleanByte > 0;
}

void PCU_status::write(uint8_t buf[8]) {
    uint8_t booleanByte;

    memcpy(&buf[0], &(message.state), sizeof(uint8_t));
    booleanByte = 0;
    if (message.bms_fault) {
        booleanByte = 1;
    }
    memcpy(&buf[1], &booleanByte, sizeof(uint8_t));
    booleanByte = 0;
    if (message.imd_fault) {
        booleanByte = 1;
    }
    memcpy(&buf[2], &booleanByte, sizeof(uint8_t));
    booleanByte = 0;
    if (message.okhs_check) {
        booleanByte = 1;
    }
    memcpy(&buf[3], &booleanByte, sizeof(uint8_t));
    booleanByte = 0;
    if (message.discharge_check) {
        booleanByte = 1;
    }
    memcpy(&buf[4], &booleanByte, sizeof(uint8_t));
}

uint8_t PCU_status::get_state() {
  return message.state;
}

bool PCU_status::get_bms_fault() {
  return message.bms_fault;
}

bool PCU_status::get_imd_fault() {
  return message.imd_fault;
}

bool PCU_status::get_okhs_check() {
    return message.okhs_check;
}

bool PCU_status::get_discharge_check() {
    return message.discharge_check;
}

void PCU_status::set_state(uint8_t state) {
  message.state = state;
}

void PCU_status::set_bms_fault(bool bms_fault) {
  message.bms_fault = bms_fault;
}

void PCU_status::set_imd_fault(bool imd_fault) {
  message.imd_fault = imd_fault;
}

void PCU_status::set_okhs_check(bool okhs_check) {
    message.okhs_check = okhs_check;
}

void PCU_status::set_discharge_check(bool discharge_check) {
    message.discharge_check = discharge_check;
}
