/*
  TCU_stats.cpp - HyTech Throttle Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech16.h"

TCU_status::TCU_status(uint8_t buf[8]) {
  update(buf);
}

void TCU_status::update(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_tcu_status_t));
}

uint8_t TCU_status::get_state() {
  return message.state;
}

uint8_t TCU_status::get_btn_start_id() {
  return message.btn_start_id;
}
