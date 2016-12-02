/*
  MC_digital_input_status.cpp - RMS Inverter CAN message: Digital Input Status
  Created by Ryan Gallaway, December 1, 2016.
 */

#include "HyTech17.h"

MC_digital_input_status::MC_digital_input_status() {
}

MC_digital_input_status::MC_digital_input_status(uint8_t buf[8]) {
  load(buf);
}

void MC_digital_input_status::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_digital_input_status_t));
}

bool MC_digital_input_status::digital_input_1() {
  return message.digital_input_1;
}

bool MC_digital_input_status::digital_input_2() {
  return message.digital_input_2;
}

bool MC_digital_input_status::digital_input_3() {
  return message.digital_input_3;
}

bool MC_digital_input_status::digital_input_4() {
  return message.digital_input_4;
}

bool MC_digital_input_status::digital_input_5() {
  return message.digital_input_5;
}

bool MC_digital_input_status::digital_input_6() {
  return message.digital_input_6;
}

bool MC_digital_input_status::digital_input_7() {
  return message.digital_input_7;
}

bool MC_digital_input_status::digital_input_8() {
  return message.digital_input_8;
}