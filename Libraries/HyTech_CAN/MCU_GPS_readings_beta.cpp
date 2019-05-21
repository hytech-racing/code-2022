/*
 * MCU_GPS_readings_beta.cpp - CAN message parser: MCU_GPS Beta message
 * Created by Nathan Cheek, May 19, 2019.
 */

#include "HyTech_CAN.h"

MCU_GPS_readings_beta::MCU_GPS_readings_beta() {
    message = {};
}

MCU_GPS_readings_beta::MCU_GPS_readings_beta(uint8_t buf[]) {
    load(buf);
}

void MCU_GPS_readings_beta::load(uint8_t buf[]) {
    message = {};
    memcpy(&(message.altitude), &buf[0], sizeof(int32_t));
    memcpy(&(message.speed), &buf[4], sizeof(int32_t));
}

void MCU_GPS_readings_beta::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.altitude), sizeof(int32_t));
    memcpy(&buf[4], &(message.speed), sizeof(int32_t));
}

int32_t MCU_GPS_readings_beta::get_altitude() {
    return message.altitude;
}

int32_t MCU_GPS_readings_beta::get_speed() {
    return message.speed;
}

void MCU_GPS_readings_beta::set_altitude(int32_t altitude) {
    message.altitude = altitude;
}

void MCU_GPS_readings_beta::set_speed(int32_t speed) {
    message.speed = speed;
}
