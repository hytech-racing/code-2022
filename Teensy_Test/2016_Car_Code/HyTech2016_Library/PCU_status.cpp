/*
  PCU_stats.cpp - HyTech Power Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech16.h"

PCU_status::PCU_status(uint8_t buf[8]) {
  update(buf);
}

void PCU_status::update(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_pcu_status_t));
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
