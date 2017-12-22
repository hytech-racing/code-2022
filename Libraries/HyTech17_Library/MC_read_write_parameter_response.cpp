/*
 * MC_read_write_parameter_response.cpp - CAN message parser: RMS Motor Controller read / write parameter response message - response from PM
 * Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_read_write_parameter_response::MC_read_write_parameter_response() {
    message = {};
}

MC_read_write_parameter_response::MC_read_write_parameter_response(uint8_t buf[8]) {
    load(buf);
}

void MC_read_write_parameter_response::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.parameter_address), &buf[0], sizeof(uint16_t));
    memcpy(&(message.write_success), &buf[2], sizeof(bool));
    memcpy(&(message.reserved1), &buf[3], sizeof(uint8_t));
    memcpy(&(message.data), &buf[4], sizeof(int32_t));
}

uint16_t MC_read_write_parameter_response::get_parameter_address() {
    return message.parameter_address;
}

bool MC_read_write_parameter_response::get_write_success() {
    return message.write_success;
}

uint32_t MC_read_write_parameter_response::get_data() {
    return message.data;
}
