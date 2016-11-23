/*
  MC_read_write_parameter_command.cpp - RMS Inverter CAN message: Read / Write Parameter Command - sent to PM
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_read_write_parameter_command::MC_read_write_parameter_command() {
}

MC_read_write_parameter_command::MC_read_write_parameter_command(uint8_t buf[8]) {
  load(buf);
}

MC_read_write_parameter_command::MC_read_write_parameter_command(uint16_t parameter_address, bool rw_command, uint16_t data) {
  set_parameter_address(parameter_address);
  set_rw_command(rw_command);
  set_data(data);
}

void MC_read_write_parameter_command::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_read_write_parameter_command_t));
}

void MC_read_write_parameter_command::write(uint8_t buf[8]) {
  memcpy(buf, &message, sizeof(CAN_message_mc_read_write_parameter_command_t));
}

uint16_t MC_read_write_parameter_command::get_parameter_address() {
  return message.parameter_address;
}

bool MC_read_write_parameter_command::get_rw_command() {
  return message.rw_command;
}

uint16_t MC_read_write_parameter_command::get_data() {
  return message.data;
}

void MC_read_write_parameter_command::set_parameter_address(uint16_t parameter_address) {
  message.parameter_address = parameter_address;
}
void MC_read_write_parameter_command::set_rw_command(bool rw_command) {
  message.rw_command = rw_command;
}
void MC_read_write_parameter_command::set_data(uint16_t data) {
  message.data = data;
}
