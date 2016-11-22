/*
  MC_fault_codes.cpp - RMS Inverter CAN message: Fault Codes
  Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech16.h"

MC_fault_codes::MC_fault_codes() {
}

MC_fault_codes::MC_fault_codes(uint8_t buf[8]) {
  load(buf);
}

void MC_fault_codes::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_fault_codes_t));
}

bool MC_fault_codes::get_post_lo_hw_gate_desaturation_fault() {
  return message.post_fault_lo && 0x0001;
}

bool MC_fault_codes::get_post_lo_hw_overcurrent_fault() {
  return message.post_fault_lo && 0x0002 >> 1;
}

bool MC_fault_codes::get_post_lo_accelerator_shorted() {
  return message.post_fault_lo && 0x0004 >> 2;
}

bool MC_fault_codes::get_post_lo_accelerator_open() {
  return message.post_fault_lo && 0x0008 >> 3;
}

bool MC_fault_codes::get_post_lo_current_sensor_low() {
  return message.post_fault_lo && 0x0010 >> 4;
}

bool MC_fault_codes::get_post_lo_current_sensor_high() {
  return message.post_fault_lo && 0x0020 >> 5;
}

bool MC_fault_codes::get_post_lo_module_temp_low() {
  return message.post_fault_lo && 0x0040 >> 6;
}

bool MC_fault_codes::get_post_lo_module_temp_high() {
  return message.post_fault_lo && 0x0080 >> 7;
}

bool MC_fault_codes::get_post_lo_ctrl_pcb_temp_low() {
  return message.post_fault_lo && 0x0100 >> 8;
}

bool MC_fault_codes::get_post_lo_ctrl_pcb_temp_high() {
  return message.post_fault_lo && 0x0200 >> 9;
}

bool MC_fault_codes::get_post_lo_gate_drive_pcb_temp_low() {
  return message.post_fault_lo && 0x0400 >> 10;
}

bool MC_fault_codes::get_post_lo_gate_drive_pcb_temp_high() {
  return message.post_fault_lo && 0x0800 >> 11;
}

bool MC_fault_codes::get_post_lo_5v_sense_voltage_low() {
  return message.post_fault_lo && 0x1000 >> 12;
}

bool MC_fault_codes::get_post_lo_5v_sense_voltage_high() {
  return message.post_fault_lo && 0x2000 >> 13;
}

bool MC_fault_codes::get_post_lo_12v_sense_voltage_low() {
  return message.post_fault_lo && 0x4000 >> 14;
}

bool MC_fault_codes::get_post_lo_12v_sense_voltage_high() {
  return message.post_fault_lo && 0x8000 >> 15;
}

bool MC_fault_codes::get_post_hi_25v_sense_voltage_low() {
  return message.post_fault_lo && 0x0001;
}

bool MC_fault_codes::get_post_hi_25v_sense_voltage_high() {
  return message.post_fault_lo && 0x0002 >> 1;
}

bool MC_fault_codes::get_post_hi_15v_sense_voltage_low() {
  return message.post_fault_lo && 0x0004 >> 2;
}

bool MC_fault_codes::get_post_hi_15v_sense_voltage_high() {
  return message.post_fault_lo && 0x0008 >> 3;
}

bool MC_fault_codes::get_post_hi_dc_bus_voltage_high() {
  return message.post_fault_lo && 0x0010 >> 4;
}

bool MC_fault_codes::get_post_hi_dc_bus_voltage_low() {
  return message.post_fault_lo && 0x0020 >> 5;
}

bool MC_fault_codes::get_post_hi_precharge_timeout() {
  return message.post_fault_lo && 0x0040 >> 6;
}

bool MC_fault_codes::get_post_hi_precharge_voltage_failure() {
  return message.post_fault_lo && 0x0080 >> 7;
}

bool MC_fault_codes::get_post_hi_eeprom_checksum_invalid() {
  return message.post_fault_lo && 0x0100 >> 8;
}

bool MC_fault_codes::get_post_hi_eeprom_data_out_of_range() {
  return message.post_fault_lo && 0x0200 >> 9;
}

bool MC_fault_codes::get_post_hi_eeprom_update_required() {
  return message.post_fault_lo && 0x0400 >> 10;
}

bool MC_fault_codes::get_post_hi_reserved1() {
  return message.post_fault_lo && 0x0800 >> 11;
}

bool MC_fault_codes::get_post_hi_reserved2() {
  return message.post_fault_lo && 0x1000 >> 12;
}

bool MC_fault_codes::get_post_hi_reserved3() {
  return message.post_fault_lo && 0x2000 >> 13;
}

bool MC_fault_codes::get_post_hi_brake_shorted() {
  return message.post_fault_lo && 0x4000 >> 14;
}

bool MC_fault_codes::get_post_hi_brake_open() {
  return message.post_fault_lo && 0x8000 >> 15;
}
