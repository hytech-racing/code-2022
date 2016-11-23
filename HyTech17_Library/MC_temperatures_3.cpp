/*
  MC_temperatures_3.cpp - RMS Inverter CAN message: Temperatures 2
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_temperatures_3::MC_temperatures_3() {
}

MC_temperatures_3::MC_temperatures_3(uint8_t buf[8]) {
  load(buf);
}

void MC_temperatures_3::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_temperatures_3_t));
}

float MC_temperatures_3::get_rtd_4_temperature() {
  return message.rtd_4_temperature / 10.0f;
}

float MC_temperatures_3::get_rtd_5_temperature() {
  return message.rtd_5_temperature / 10.0f;
}

float MC_temperatures_3::get_motor_temperature() {
  return message.motor_temperature / 10.0f;
}

float MC_temperatures_3::get_torque_shudder() {
  return message.torque_shudder / 10.0f;
}
