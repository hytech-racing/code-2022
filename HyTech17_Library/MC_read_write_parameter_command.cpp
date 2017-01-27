/*
  MC_read_write_parameter_command.cpp - RMS Inverter CAN message: Read / Write Parameter Command - sent to PM
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_read_write_parameter_command::MC_read_write_parameter_command() {
    message = {};
}

MC_read_write_parameter_command::MC_read_write_parameter_command(uint8_t buf[8]) {
  load(buf);
}

MC_read_write_parameter_command::MC_read_write_parameter_command(uint16_t parameter_address, bool rw_command, uint8_t data[]) {
  set_parameter_address(parameter_address);
  set_rw_command(rw_command);
  set_data(data);
}

void MC_read_write_parameter_command::load(uint8_t buf[8]) {
    message = {};
    int index = 0;
    memcpy(&(message.parameter_address), buf + index, sizeof(uint16_t));
    index += sizeof(uint16_t);
    uint8_t booleanByte = 0;
    memcpy(&booleanByte, buf + index, sizeof(uint8_t));
    message.rw_command = booleanByte > 0;
    index += sizeof(uint8_t);
    memcpy(&(message.reserved1), buf + index, sizeof(uint8_t));
    index += sizeof(uint8_t);
    memcpy(message.data, buf + index, sizeof(int));
}

void MC_read_write_parameter_command::write(uint8_t buf[8]) {
    int index = 0;
    memcpy(buf + index, &(message.parameter_address), sizeof(uint16_t));
    index += sizeof(uint16_t);
    uint8_t booleanByte = 0;
    if (message.rw_command) {
        booleanByte = true;
    }
    memcpy(buf + index, &booleanByte, sizeof(uint8_t));
    index += sizeof(uint8_t);
    // memcpy(&(message.reserved1), buf + index, sizeof(uint8_t));
    index += sizeof(uint8_t);
    memcpy(buf + index, &message.data, sizeof(int));
}

uint16_t MC_read_write_parameter_command::get_parameter_address() {
  return message.parameter_address;
}

bool MC_read_write_parameter_command::get_rw_command() {
  return message.rw_command;
}

uint8_t* MC_read_write_parameter_command::get_data() {
  return message.data;
}

void MC_read_write_parameter_command::set_parameter_address(uint16_t parameter_address) {
  message.parameter_address = parameter_address;
}
void MC_read_write_parameter_command::set_rw_command(bool rw_command) {
  message.rw_command = rw_command;
}
void MC_read_write_parameter_command::set_data(uint8_t data[]) {
  memcpy(message.data, data, sizeof(int));
}
