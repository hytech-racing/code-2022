/*
 * MCU_GPS_readings_alpha.cpp - CAN message parser: MCU_GPS Alpha message
 * Created by Nathan Cheek, May 19, 2019.
 */

#include "HyTech_CAN.h"

MCU_GPS_readings_alpha::MCU_GPS_readings_alpha() {
    message = {};
}

MCU_GPS_readings_alpha::MCU_GPS_readings_alpha(uint8_t buf[]) {
    load(buf);
}

void MCU_GPS_readings_alpha::load(uint8_t buf[]) {
    message = {};
    memcpy(&(message.latitude), &buf[0], sizeof(int32_t));
    memcpy(&(message.longitude), &buf[4], sizeof(int32_t));
}

void MCU_GPS_readings_alpha::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.latitude), sizeof(int32_t));
    memcpy(&buf[4], &(message.longitude), sizeof(int32_t));
}

int32_t MCU_GPS_readings_alpha::get_latitude() {
    return message.latitude;
}

int32_t MCU_GPS_readings_alpha::get_longitude() {
    return message.longitude;
}

void MCU_GPS_readings_alpha::set_latitude(int32_t latitude) {
    message.latitude = latitude;
}

void MCU_GPS_readings_alpha::set_longitude(int32_t longitude) {
    message.longitude = longitude;
}
