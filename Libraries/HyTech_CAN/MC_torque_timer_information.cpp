/*
 * MC_torque_timer_information.cpp - CAN message parser: RMS Motor Controller torque timer information message
 * Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech_CAN.h"

MC_torque_timer_information::MC_torque_timer_information() {
    message = {};
}

MC_torque_timer_information::MC_torque_timer_information(uint8_t buf[8]) {
    load(buf);
}

void MC_torque_timer_information::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.commanded_torque), &buf[0], sizeof(int16_t));
    memcpy(&(message.torque_feedback), &buf[2], sizeof(int16_t));
    memcpy(&(message.power_on_timer), &buf[4], sizeof(uint32_t));
}

void MC_torque_timer_information::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.commanded_torque), sizeof(int16_t));
    memcpy(&buf[2], &(message.torque_feedback), sizeof(int16_t));
    memcpy(&buf[4], &(message.power_on_timer), sizeof(uint32_t));
}

int16_t MC_torque_timer_information::get_commanded_torque() {
    return message.commanded_torque;
}

int16_t MC_torque_timer_information::get_torque_feedback() {
    return message.torque_feedback;
}

uint32_t MC_torque_timer_information::get_power_on_timer() {
    return message.power_on_timer;
}
