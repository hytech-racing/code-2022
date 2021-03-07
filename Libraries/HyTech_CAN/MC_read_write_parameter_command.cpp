/*
 * MC_read_write_parameter_command.cpp - CAN message parser: RMS Motor Controller read / write parameter command message - sent to PM
 * Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech_CAN.h"

MC_read_write_parameter_command::MC_read_write_parameter_command() {
    message = {};
}

MC_read_write_parameter_command::MC_read_write_parameter_command(uint8_t buf[8]) {
    load(buf);
}

MC_read_write_parameter_command::MC_read_write_parameter_command(uint16_t parameter_address, bool rw_command, uint32_t data) {
    message = {};
    set_parameter_address(parameter_address);
    set_rw_command(rw_command);
    set_data(data);
}

void MC_read_write_parameter_command::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.parameter_address), &buf[0], sizeof(uint16_t));
    memcpy(&(message.rw_command), &buf[2], sizeof(bool));
    memcpy(&(message.reserved1), &buf[3], sizeof(uint8_t));
    memcpy(&(message.data), &buf[4], sizeof(int32_t));
}

void MC_read_write_parameter_command::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.parameter_address), sizeof(uint16_t));
    memcpy(&buf[2], &(message.rw_command), sizeof(bool));
    memcpy(&buf[3], &(message.reserved1), sizeof(uint8_t));
    memcpy(&buf[4], &(message.data), sizeof(int32_t));
}

uint16_t MC_read_write_parameter_command::get_parameter_address() {
    return message.parameter_address;
}

bool MC_read_write_parameter_command::get_rw_command() {
    return message.rw_command;
}

uint32_t MC_read_write_parameter_command::get_data() {
    return message.data;
}

void MC_read_write_parameter_command::set_parameter_address(uint16_t parameter_address) {
    message.parameter_address = parameter_address;
}
void MC_read_write_parameter_command::set_rw_command(bool rw_command) {
    message.rw_command = rw_command;
}
void MC_read_write_parameter_command::set_data(uint32_t data) {
    message.data = data;
}
