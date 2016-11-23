/*
  MC_torque_timer_information.cpp - RMS Inverter CAN message: Torque Timer Information
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_torque_timer_information::MC_torque_timer_information() {
}

MC_torque_timer_information::MC_torque_timer_information(uint8_t buf[8]) {
  load(buf);
}

void MC_torque_timer_information::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_torque_timer_information_t));
}

float MC_torque_timer_information::get_commanded_torque() {
  return message.commanded_torque / 10.0f;
}

float MC_torque_timer_information::get_torque_feedback() {
  return message.torque_feedback / 10.0f;
}

uint32_t MC_torque_timer_information::get_power_on_timer() {
  return message.power_on_timer;
}
