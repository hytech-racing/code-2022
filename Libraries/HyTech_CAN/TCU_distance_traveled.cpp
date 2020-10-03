/*
 * TCU_distance_traveled.cpp - CAN message parser: Total distance traveled
 * Created by Peter Wagstaff, March 1, 2020.
 */

#include "HyTech_CAN.h"

TCU_distance_traveled::TCU_distance_traveled() {
    message = {};
}

TCU_distance_traveled::TCU_distance_traveled(uint8_t buf[8]) {
    load(buf);
}

void TCU_distance_traveled::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.distance_traveled), &buf[0], sizeof(int16_t));
}

void TCU_distance_traveled::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.distance_traveled), sizeof(int16_t));
}

int16_t TCU_distance_traveled::get_distance_traveled() {
    return message.distance_traveled;
}

void TCU_distance_traveled::set_distance_traveled(uint16_t value) {
    message.distance_traveled = value;
}
