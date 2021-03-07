/*
 * MC_motor_position_information.cpp - CAN message parser: RMS Motor Controller motor position information message
 * Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech_CAN.h"

MC_motor_position_information::MC_motor_position_information() {
    message = {};
}

MC_motor_position_information::MC_motor_position_information(uint8_t buf[8]) {
    load(buf);
}

void MC_motor_position_information::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.motor_angle), &buf[0], sizeof(int16_t));
    memcpy(&(message.motor_speed), &buf[2], sizeof(int16_t));
    memcpy(&(message.electrical_output_frequency), &buf[4], sizeof(int16_t));
    memcpy(&(message.delta_resolver_filtered), &buf[6], sizeof(int16_t));
}

void MC_motor_position_information::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.motor_angle), sizeof(int16_t));
    memcpy(&buf[2], &(message.motor_speed), sizeof(int16_t));
    memcpy(&buf[4], &(message.electrical_output_frequency), sizeof(int16_t));
    memcpy(&buf[6], &(message.delta_resolver_filtered), sizeof(int16_t));
}

int16_t MC_motor_position_information::get_motor_angle() {
    return message.motor_angle;
}

int16_t MC_motor_position_information::get_motor_speed() {
    return message.motor_speed;
}

int16_t MC_motor_position_information::get_electrical_output_frequency() {
    return message.electrical_output_frequency;
}

int16_t MC_motor_position_information::get_delta_resolver_filtered() {
    return message.delta_resolver_filtered;
}
