/*
  MC_current_information.cpp - RMS Inverter CAN message: Voltage Information
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech17.h"

MC_current_information::MC_current_information() {
    message = {};
}

MC_current_information::MC_current_information(uint8_t buf[8]) {
  load(buf);
}

void MC_current_information::load(uint8_t buf[8]) {
    message = {};
    int index = 0;
    memcpy(&(message.phase_a_current), buf + index, sizeof(int16_t));
    index += sizeof(int16_t);
    memcpy(&(message.phase_b_current), buf + index, sizeof(int16_t));
    index += sizeof(int16_t);
    memcpy(&(message.phase_c_current), buf + index, sizeof(int16_t));
    index += sizeof(int16_t);
    memcpy(&(message.dc_bus_current), buf + index, sizeof(int16_t));
}

int16_t MC_current_information::get_phase_a_current() {
  return message.phase_a_current;
}

int16_t MC_current_information::get_phase_b_current() {
  return message.phase_b_current;
}

int16_t MC_current_information::get_phase_c_current() {
  return message.phase_c_current;
}

int16_t MC_current_information::get_dc_bus_current() {
  return message.dc_bus_current;
}
