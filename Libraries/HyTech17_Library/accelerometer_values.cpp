// Created by Soohyun Kim
// For packaging and sending accelerometer values over CAN

#include "HyTech17.h"

accelerometer_values::accelerometer_values() {
   message = {};
}

accelerometer_values::accelerometer_values(uint8_t buf[8]) {
   load(buf);
}

short accelerometer_values::getX() {
   return message.XValue_x100;
}

short accelerometer_values::getY() {
   return message.YValue_x100;
}

short accelerometer_values::getZ() {
   return message.ZValue_x100;
}

void accelerometer_values::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.XValue_x100), sizeof(short));
    memcpy(&buf[2], &(message.YValue_x100), sizeof(short));
    memcpy(&buf[4], &(message.ZValue_x100), sizeof(short));
}

void accelerometer_values::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.XValue_x100), &buf[0], sizeof(short));
    memcpy(&(message.YValue_x100), &buf[2], sizeof(short));
    memcpy(&(message.ZValue_x100), &buf[4], sizeof(short));
}

void accelerometer_values::setValues(short x, short y, short z) {
    message = {};
    message.XValue_x100 = x;
    message.YValue_x100 = y;
    message.ZValue_x100 = z;
}