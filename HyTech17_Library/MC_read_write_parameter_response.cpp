/*
  MC_read_write_parameter_response.cpp - RMS Inverter CAN message: Read / Write Parameter Response - response from PM
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_read_write_parameter_response::MC_read_write_parameter_response() {
}

MC_read_write_parameter_response::MC_read_write_parameter_response(uint8_t buf[8]) {
  load(buf);
}

void MC_read_write_parameter_response::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_read_write_parameter_response_t));
}

uint16_t MC_read_write_parameter_response::get_parameter_address() {
  return message.parameter_address;
}

bool MC_read_write_parameter_response::get_write_success() {
  return message.write_success;
}

uint16_t MC_read_write_parameter_response::get_data() {
  return message.data;
}
