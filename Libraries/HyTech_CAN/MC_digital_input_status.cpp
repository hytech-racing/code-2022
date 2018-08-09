/*
 * MC_digital_input_status.cpp - CAN message parser: RMS Motor Controller digital input status message
 * Created by Ryan Gallaway, December 1, 2016.
 */

#include "HyTech_CAN.h"

MC_digital_input_status::MC_digital_input_status() {
    message = {};
}

MC_digital_input_status::MC_digital_input_status(uint8_t buf[]) {
    load(buf);
}

void MC_digital_input_status::load(uint8_t buf[]) {
    message = {};
    memcpy(&(message.digital_input_1), &buf[0], sizeof(bool));
    memcpy(&(message.digital_input_2), &buf[1], sizeof(bool));
    memcpy(&(message.digital_input_3), &buf[2], sizeof(bool));
    memcpy(&(message.digital_input_4), &buf[3], sizeof(bool));
    memcpy(&(message.digital_input_5), &buf[4], sizeof(bool));
    memcpy(&(message.digital_input_6), &buf[5], sizeof(bool));
    memcpy(&(message.digital_input_7), &buf[6], sizeof(bool));
    memcpy(&(message.digital_input_8), &buf[7], sizeof(bool));
}

void MC_digital_input_status::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.digital_input_1), sizeof(bool));
    memcpy(&buf[1], &(message.digital_input_2), sizeof(bool));
    memcpy(&buf[2], &(message.digital_input_3), sizeof(bool));
    memcpy(&buf[3], &(message.digital_input_4), sizeof(bool));
    memcpy(&buf[4], &(message.digital_input_5), sizeof(bool));
    memcpy(&buf[5], &(message.digital_input_6), sizeof(bool));
    memcpy(&buf[6], &(message.digital_input_7), sizeof(bool));
    memcpy(&buf[7], &(message.digital_input_8), sizeof(bool));
}

bool MC_digital_input_status::get_digital_input_1() {
    return message.digital_input_1;
}

bool MC_digital_input_status::get_digital_input_2() {
    return message.digital_input_2;
}

bool MC_digital_input_status::get_digital_input_3() {
    return message.digital_input_3;
}

bool MC_digital_input_status::get_digital_input_4() {
    return message.digital_input_4;
}

bool MC_digital_input_status::get_digital_input_5() {
    return message.digital_input_5;
}

bool MC_digital_input_status::get_digital_input_6() {
    return message.digital_input_6;
}

bool MC_digital_input_status::get_digital_input_7() {
    return message.digital_input_7;
}

bool MC_digital_input_status::get_digital_input_8() {
    return message.digital_input_8;
}