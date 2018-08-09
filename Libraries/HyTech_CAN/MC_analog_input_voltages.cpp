/*
 * MC_analog_input_voltages.cpp - CAN message parser: RMS Motor Controller analog input voltages message
 * Created by Ryan Gallaway, December 1, 2016.
 */

#include "HyTech_CAN.h"

MC_analog_input_voltages::MC_analog_input_voltages() {
    message = {};
}

MC_analog_input_voltages::MC_analog_input_voltages(uint8_t buf[8]) {
    load(buf);
}

void MC_analog_input_voltages::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.analog_input_1), &buf[0], sizeof(int16_t));
    memcpy(&(message.analog_input_2), &buf[2], sizeof(int16_t));
    memcpy(&(message.analog_input_3), &buf[4], sizeof(int16_t));
    memcpy(&(message.analog_input_4), &buf[6], sizeof(int16_t));
}

void MC_analog_input_voltages::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.analog_input_1), sizeof(int16_t));
    memcpy(&buf[2], &(message.analog_input_2), sizeof(int16_t));
    memcpy(&buf[4], &(message.analog_input_3), sizeof(int16_t));
    memcpy(&buf[6], &(message.analog_input_4), sizeof(int16_t));
}

int16_t MC_analog_input_voltages::get_analog_input_1() {
    return message.analog_input_1;
}

int16_t MC_analog_input_voltages::get_analog_input_2() {
    return message.analog_input_2;
}

int16_t MC_analog_input_voltages::get_analog_input_3() {
    return message.analog_input_3;
}

int16_t MC_analog_input_voltages::get_analog_input_4() {
    return message.analog_input_4;
}
