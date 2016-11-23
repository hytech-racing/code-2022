/*
  MC_temperatures_2.cpp - RMS Inverter CAN message: Temperatures 2
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_temperatures_2::MC_temperatures_2() {
}

MC_temperatures_2::MC_temperatures_2(uint8_t buf[8]) {
  load(buf);
}

void MC_temperatures_2::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_temperatures_2_t));
}

float MC_temperatures_2::get_control_board_temperature() {
  return message.control_board_temperature / 10.0f;
}

float MC_temperatures_2::get_rtd_1_temperature() {
  return message.rtd_1_temperature / 10.0f;
}

float MC_temperatures_2::get_rtd_2_temperature() {
  return message.rtd_2_temperature / 10.0f;
}

float MC_temperatures_2::get_rtd_3_temperature() {
  return message.rtd_3_temperature / 10.0f;
}
