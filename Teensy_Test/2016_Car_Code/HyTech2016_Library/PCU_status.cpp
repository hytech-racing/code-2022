/*
  PCU_stats.cpp - HyTech Power Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech16.h"

PCU_status::PCU_status(uint8_t buf[8]) {
  set(buf);
}

void PCU_status::get(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_pcu_status_t));
}

void PCU_status::set(uint8_t buf[8]) {
  memcpy(buf, &message, sizeof(CAN_message_pcu_status_t));
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

void PCU_status::set_state(uint8_t state) {
  message.state = state;
}

void PCU_status::set_bms_fault(bool bms_fault) {
  message.bms_fault = bms_fault;
}

void PCU_status::set_imd_fault(bool imd_fault) {
  message.imd_fault = imd_fault;
}
