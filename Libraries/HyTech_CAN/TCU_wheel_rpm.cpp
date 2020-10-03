/*
 * TCU_wheel_rpm.cpp - CAN message parser: Wheel rpm sensor information message
 * Created by Nathan Cheek, November 22, 2016.
 */

#include "HyTech_CAN.h"

TCU_wheel_rpm::TCU_wheel_rpm() {
    message = {};
}

TCU_wheel_rpm::TCU_wheel_rpm(uint8_t buf[8]) {
    load(buf);
}

void TCU_wheel_rpm::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.wheel_rpm_left), &buf[0], sizeof(int16_t));
    memcpy(&(message.wheel_rpm_right), &buf[2], sizeof(int16_t));
}

void TCU_wheel_rpm::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.wheel_rpm_left), sizeof(int16_t));
    memcpy(&buf[2], &(message.wheel_rpm_right), sizeof(int16_t));
}

int16_t TCU_wheel_rpm::get_wheel_rpm_left() {
    return message.wheel_rpm_left;
}

int16_t TCU_wheel_rpm::get_wheel_rpm_right() {
    return message.wheel_rpm_right;
}

void TCU_wheel_rpm::set_wheel_rpm_left(uint16_t value) {
    message.wheel_rpm_left = value;
}

void TCU_wheel_rpm::set_wheel_rpm_right(uint16_t value) {
    message.wheel_rpm_right = value;
}
