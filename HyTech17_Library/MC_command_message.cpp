/*
  MC_command_message.cpp - HyTech Throttle Control Unit CAN message: Status
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech17.h"

MC_command_message::MC_command_message() {
}

MC_command_message::MC_command_message(uint8_t buf[8]) {
  load(buf);
}

MC_command_message::MC_command_message(int16_t torque_command, int16_t angular_velocity, bool direction, bool inverter_enable, bool discharge_enable, int16_t commanded_torque_limit) {
  set_torque_command(torque_command);
  set_angular_velocity(angular_velocity);
  set_direction(direction);
  set_inverter_enable(inverter_enable);
  set_discharge_enable(discharge_enable);
  set_commanded_torque_limit(commanded_torque_limit);
}

void MC_command_message::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_command_message_t));
}

void MC_command_message::write(uint8_t buf[8]) {
  memcpy(buf, &message, sizeof(CAN_message_mc_command_message_t));
}

int8_t MC_command_message::get_torque_command() {
  return message.torque_command;
}

int8_t MC_command_message::get_angular_velocity() {
  return message.angular_velocity;
}

bool MC_command_message::get_direction() {
  return message.direction;
}

bool MC_command_message::get_inverter_enable() {
  return message.inverter_enable_discharge_enable & 0x80 >> 7;
}

bool MC_command_message::get_discharge_enable() {
  return message.inverter_enable_discharge_enable & 0x40 >> 6;
}

int16_t MC_command_message::get_commanded_torque_limit() {
  return message.commanded_torque_limit;
}

void MC_command_message::set_torque_command(int16_t torque_command) {
  message.torque_command = torque_command;
}

void MC_command_message::set_angular_velocity(int16_t angular_velocity) {
  message.angular_velocity = angular_velocity;
}

void MC_command_message::set_direction(bool direction) {
  message.direction = direction;
}

void MC_command_message::set_inverter_enable(bool inverter_enable) {
  message.inverter_enable = (message.inverter_enable & 0x7F) | (inverter_enable << 7);
}

void MC_command_message::set_discharge_enable(bool discharge_enable) {
  message.discharge_enable = (message.discharge_enable & 0xBF) | (inverter_enable & 0x1 << 6);
}

void MC_command_message::set_commanded_torque_limit(int16_t commanded_torque_limit) {
  message.commanded_torque_limit = commanded_torque_limit;
}
