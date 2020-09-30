/*
 * MCU_launch_control.cpp - CAN message parser: Launch Control information message
 * Created by Peter Wagstaff, March 1, 2020.
 */

#include "HyTech_CAN.h"

MCU_launch_control::MCU_launch_control() {
    message = {};
}

MCU_launch_control::MCU_launch_control(uint8_t buf[8]) {
    load(buf);
}

void MCU_launch_control::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.slip_ratio), &buf[0], sizeof(int16_t));
    memcpy(&(message.slip_limiting_factor), &buf[2], sizeof(int16_t));
}

void MCU_launch_control::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.slip_ratio), sizeof(int16_t));
    memcpy(&buf[2], &(message.slip_limiting_factor), sizeof(int16_t));
}

int16_t MCU_launch_control::get_slip_ratio() {
    return message.slip_ratio;
}

int16_t MCU_launch_control::get_slip_limiting_factor() {
    return message.slip_limiting_factor;
}

void MCU_launch_control::set_slip_ratio(uint16_t value) {
    message.slip_ratio = value;
}

void MCU_launch_control::set_slip_limiting_factor(uint16_t value) {
    message.slip_limiting_factor = value;
}
