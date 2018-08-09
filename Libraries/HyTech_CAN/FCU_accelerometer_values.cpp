/*
 * FCU_accelerometer_values.cpp - retrieving and packaging accelerometer values via CAN
 * Created by Soohyun Kim, July 12, 2018.
 */

#include "HyTech_CAN.h"

FCU_accelerometer_values::FCU_accelerometer_values() {
   message = {};
}

FCU_accelerometer_values::FCU_accelerometer_values(uint8_t buf[8]) {
   load(buf);
}

void FCU_accelerometer_values::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.XValue_x100), &buf[0], sizeof(short));
    memcpy(&(message.YValue_x100), &buf[2], sizeof(short));
    memcpy(&(message.ZValue_x100), &buf[4], sizeof(short));
}

void FCU_accelerometer_values::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.XValue_x100), sizeof(short));
    memcpy(&buf[2], &(message.YValue_x100), sizeof(short));
    memcpy(&buf[4], &(message.ZValue_x100), sizeof(short));
}

short FCU_accelerometer_values::get_x() {
   return message.XValue_x100;
}

short FCU_accelerometer_values::get_y() {
   return message.YValue_x100;
}

short FCU_accelerometer_values::get_z() {
   return message.ZValue_x100;
}

void FCU_accelerometer_values::set_values(short x, short y, short z) {
    message = {};
    message.XValue_x100 = x;
    message.YValue_x100 = y;
    message.ZValue_x100 = z;
}