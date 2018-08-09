/*
 * MC_modulation_index_flux_weakening_output_information.cpp - CAN message parser: RMS Motor Controller modulation index & flux weakening output information message
 * Created by Ryan Gallaway, December 1, 2016.
 */

#include "HyTech_CAN.h"

MC_modulation_index_flux_weakening_output_information::MC_modulation_index_flux_weakening_output_information() {
    message = {};
}

MC_modulation_index_flux_weakening_output_information::MC_modulation_index_flux_weakening_output_information(uint8_t buf[8]) {
    load(buf);
}

void MC_modulation_index_flux_weakening_output_information::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.modulation_index), &buf[0], sizeof(uint16_t));
    memcpy(&(message.flux_weakening_output), &buf[2], sizeof(int16_t));
    memcpy(&(message.id_command), &buf[4], sizeof(int16_t));
    memcpy(&(message.iq_command), &buf[6], sizeof(int16_t));
}

void MC_modulation_index_flux_weakening_output_information::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.modulation_index), sizeof(uint16_t));
    memcpy(&buf[2], &(message.flux_weakening_output), sizeof(int16_t));
    memcpy(&buf[4], &(message.id_command), sizeof(int16_t));
    memcpy(&buf[6], &(message.iq_command), sizeof(int16_t));
}

uint16_t MC_modulation_index_flux_weakening_output_information::get_modulation_index() {
    return message.modulation_index;
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
