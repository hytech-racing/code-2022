/*
  MC_modulation_index_flux_weakening_output_information.cpp - RMS Inverter CAN message:
    Modulation Index & Flux Weakening Output Information
  Created by Ryan Gallaway, December 1, 2016.
 */

#include "HyTech17.h"

MC_modulation_index_flux_weakening_output_information::MC_modulation_index_flux_weakening_output_information() {
}

MC_modulation_index_flux_weakening_output_information::MC_modulation_index_flux_weakening_output_information(uint8_t buf[8]) {
  load(buf);
}

void MC_modulation_index_flux_weakening_output_information::load(uint8_t buf[8]) {
  memcpy(&message, buf, sizeof(CAN_message_mc_modulation_index_flux_weakening_output_information_t));
}

uint16_t MC_modulation_index_flux_weakening_output_information::get_modulation_index() {
  return message.modulaton_index;
}

int16_t MC_modulation_index_flux_weakening_output_information::get_flux_weakening_output() {
  return message.flux_weakening_output;
}

int16_t MC_modulation_index_flux_weakening_output_information::get_id_command() {
  return message.id_command;
}

int16_t MC_modulation_index_flux_weakening_output_information::get_iq_command() {
  return message.iq_command;
}