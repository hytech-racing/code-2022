/*
 * MCU_GPS_readings_gamma.cpp - CAN message parser: MCU_GPS Gamma message
 * Created by Nathan Cheek, May 19, 2019.
 */

#include "HyTech_CAN.h"

MCU_GPS_readings_gamma::MCU_GPS_readings_gamma() {
    message = {};
}

MCU_GPS_readings_gamma::MCU_GPS_readings_gamma(uint8_t buf[]) {
    load(buf);
}

void MCU_GPS_readings_gamma::load(uint8_t buf[]) {
    message = {};
    memcpy(&(message.fix_quality), &buf[0], sizeof(uint8_t));
    memcpy(&(message.satellite_count), &buf[1], sizeof(uint8_t));
    memcpy(&(message.timestamp_seconds), &buf[2], sizeof(uint32_t));
    memcpy(&(message.timestamp_milliseconds), &buf[6], sizeof(uint16_t));    
}

void MCU_GPS_readings_gamma::write(uint8_t buf[]) {
    memcpy(&buf[0], &(message.fix_quality), sizeof(uint8_t));
    memcpy(&buf[1], &(message.satellite_count), sizeof(uint8_t));
    memcpy(&buf[2], &(message.timestamp_seconds), sizeof(uint32_t));
    memcpy(&buf[6], &(message.timestamp_milliseconds), sizeof(uint16_t));
}

uint8_t MCU_GPS_readings_gamma::get_fix_quality() {
    return message.fix_quality;
}

uint8_t MCU_GPS_readings_gamma::get_satellite_count() {
    return message.satellite_count;
}

uint32_t MCU_GPS_readings_gamma::get_timestamp_seconds() {
    return message.timestamp_seconds;
}

uint16_t MCU_GPS_readings_gamma::get_timestamp_milliseconds() {
    return message.timestamp_milliseconds;
}

void MCU_GPS_readings_gamma::set_fix_quality(uint8_t fix_quality) {
    message.fix_quality = fix_quality;
}

void MCU_GPS_readings_gamma::set_satellite_count(uint8_t satellite_count) {
    message.satellite_count = satellite_count;
}

void MCU_GPS_readings_gamma::set_timestamp_seconds(uint32_t timestamp_seconds) {
    message.timestamp_seconds = timestamp_seconds;
}

void MCU_GPS_readings_gamma::set_timestamp_milliseconds(uint16_t timestamp_milliseconds) {
    message.timestamp_milliseconds = timestamp_milliseconds;
}