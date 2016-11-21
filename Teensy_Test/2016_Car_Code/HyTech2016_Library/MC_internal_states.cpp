/*
  MC_internal_states.cpp - RMS Inverter CAN message: Internal States
  Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech16.h"

MC_internal_states::MC_internal_states(uint8_t buf[8]) {
  update(buf);
}

void MC_internal_states::update(uint8_t buf[8]) {
  memcpy(buf, &message, sizeof(CAN_message_mc_internal_states_t));
}

uint8_t MC_internal_states::get_vsm_state() {
  return message.vsm_state;
}

uint8_t MC_internal_states::get_inverter_state() {
  return message.inverter_state;
}

bool MC_internal_states::get_relay_active_1() {
  return RELAY_STATE_1(message.relay_state);
}

bool MC_internal_states::get_relay_active_2() {
  return RELAY_STATE_2(message.relay_state);
}

bool MC_internal_states::get_relay_active_3() {
  return RELAY_STATE_3(message.relay_state);
}

bool MC_internal_states::get_relay_active_4() {
  return RELAY_STATE_4(message.relay_state);
}

bool MC_internal_states::get_relay_active_5() {
  return RELAY_STATE_5(message.relay_state);
}

bool MC_internal_states::get_relay_active_6() {
  return RELAY_STATE_6(message.relay_state);
}

bool MC_internal_states::get_inverter_run_mode() {
  return INVERTER_RUN_MODE(message.inverter_run_mode_discharge_state);
}

uint8_t MC_internal_states::get_inverter_active_discharge_state() {
  return INVERTER_ACTIVE_DISCHARGE_STATE(message.inverter_run_mode_discharge_state);
}

bool MC_internal_states::get_inverter_command_mode() {
  return message.inverter_command_mode;
}

bool MC_internal_states::get_inverter_enable_state() {
  return INVERTER_ENABLE_STATE(message.inverter_enable);
}

bool MC_internal_states::get_inverter_enable_lockout() {
  return INVERTER_ENABLE_LOCKOUT(message.inverter_enable);
}

bool MC_internal_states::get_direction_command() {
  return message.direction_command;
}
