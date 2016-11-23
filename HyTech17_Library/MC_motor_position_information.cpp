/*
  MC_motor_position_information.cpp - RMS Inverter CAN message: Motor Position Information
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech17.h"

MC_motor_position_information::MC_motor_position_information() {
}

MC_motor_position_information::MC_motor_position_information(uint8_t buf[8]) {
  load(buf);
}

void MC_motor_position_information::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_motor_position_information_t));
}

float MC_motor_position_information::get_motor_angle() {
  return message.motor_angle / 10.0f;
}

int16_t MC_motor_position_information::get_motor_speed() {
  return message.motor_speed;
}

float MC_motor_position_information::get_electrical_output_frequency() {
  return message.electrical_output_frequency / 10.0f;
}

float MC_motor_position_information::get_delta_resolver_filtered() {
  return message.delta_resolver_filtered / 10.0f;
}
