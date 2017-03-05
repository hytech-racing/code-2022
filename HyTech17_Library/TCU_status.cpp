/*
  TCU_status.cpp - HyTech Throttle Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
  TODO: Make sure functions are updated
 */

#include "HyTech17.h"

TCU_status::TCU_status() {
    message = {};
}

TCU_status::TCU_status(uint8_t buf[8]) {
  load(buf);
}

TCU_status::TCU_status(uint8_t state, uint8_t btn_start_id) {
  set_state(state);
  set_btn_start_id(btn_start_id);
}

void TCU_status::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.state), &buf[0], sizeof(uint8_t));
    memcpy(&(message.btn_start_id), &buf[1], sizeof(uint8_t));
}

void TCU_status::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.state), sizeof(uint8_t));
    memcpy(&buf[1], &(message.btn_start_id), sizeof(uint8_t));
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
