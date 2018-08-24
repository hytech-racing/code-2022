/*
 * MC_current_information.cpp - CAN message parser: RMS Motor Controller voltage information message
 * Created by Nathan Cheek, November 20, 2016.
 */

#include "HyTech_CAN.h"

MC_current_information::MC_current_information() {
    message = {};
}

MC_current_information::MC_current_information(uint8_t buf[8]) {
    load(buf);
}

void MC_current_information::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.phase_a_current), &buf[0], sizeof(int16_t));
    memcpy(&(message.phase_b_current), &buf[2], sizeof(int16_t));
    memcpy(&(message.phase_c_current), &buf[4], sizeof(int16_t));
    memcpy(&(message.dc_bus_current), &buf[6], sizeof(int16_t));
}

void MC_current_information::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.phase_a_current), sizeof(int16_t));
    memcpy(&buf[2], &(message.phase_b_current), sizeof(int16_t));
    memcpy(&buf[4], &(message.phase_c_current), sizeof(int16_t));
    memcpy(&buf[6], &(message.dc_bus_current), sizeof(int16_t));
}

int16_t MC_current_information::get_phase_a_current() {
    return message.phase_a_current;
}

int16_t MC_current_information::get_phase_b_current() {
    return message.phase_b_current;
}

int16_t MC_current_information::get_phase_c_current() {
    return message.phase_c_current;
}

int16_t MC_current_information::get_dc_bus_current() {
    return message.dc_bus_current;
}
