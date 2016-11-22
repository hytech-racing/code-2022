/*
  MC_voltage_information.cpp - RMS Inverter CAN message: Voltage Information
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech16.h"

MC_voltage_information::MC_voltage_information() {
}

MC_voltage_information::MC_voltage_information(uint8_t buf[8]) {
  load(buf);
}

void MC_voltage_information::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_voltage_information_t));
}

float MC_voltage_information::get_dc_bus_voltage() {
  return message.dc_bus_voltage / 10.0f;
}

float MC_voltage_information::get_output_voltage() {
  return message.output_voltage / 10.0f;
}

float MC_voltage_information::get_phase_ab_voltage() {
  return message.phase_ab_voltage / 10.0f;
}

float MC_voltage_information::get_phase_bc_voltage() {
  return message.phase_bc_voltage / 10.0f;
}
