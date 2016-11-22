/*
  TCU_status.cpp - HyTech Throttle Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech16.h"

TCU_status::TCU_status() {
}

TCU_status::TCU_status(uint8_t buf[8]) {
  load(buf);
}

void TCU_status::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_tcu_status_t));
}

void TCU_status::write(uint8_t buf[8]) {
  memcpy(buf, &message, sizeof(CAN_message_tcu_status_t));
}

uint8_t TCU_status::get_state() {
  return message.state;
}

uint8_t TCU_status::get_btn_start_id() {
  return message.btn_start_id;
}

void TCU_status::set_state(uint8_t state) {
  message.state = state;
}

void TCU_status::set_btn_start_id(uint8_t btn_start_id) {
  message.btn_start_id = btn_start_id;
}
