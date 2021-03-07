#include "HyTech_CAN.h"

//Constructors

GLV_current_readings::GLV_current_readings(){
    message = {};
}

GLV_current_readings::GLV_current_readings(uint8_t buf[8]) {
    load(buf);
}

GLV_current_readings::GLV_current_readings(uint16_t ecu_current_value, uint16_t cooling_current_value) {
    set_ecu_current_value(ecu_current_value);
    set_cooling_current_value(cooling_current_value);
}

//Load from buffer & write to variable instance

void GLV_current_readings::load(uint8_t buf[8]) {
    message = {};

    memcpy(&(message.ecu_current_value), &buf[0], sizeof(uint16_t));
    memcpy(&(message.cooling_current_value), &buf[2], sizeof(uint16_t));
}

//Write to buffer

void GLV_current_readings::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.ecu_current_value), sizeof(uint16_t));
    memcpy(&buf[2], &(message.cooling_current_value), sizeof(uint16_t));
}

//Get Functions

uint16_t GLV_current_readings::get_ecu_current_value() {
    return message.ecu_current_value;
}

uint16_t GLV_current_readings::get_cooling_current_value() {
    return message.cooling_current_value;
}

// Set functions

void GLV_current_readings::set_ecu_current_value(uint16_t ecu_current_value) {
    message.ecu_current_value = ecu_current_value;
}

void GLV_current_readings::set_cooling_current_value(uint16_t cooling_current_value) {
    message.cooling_current_value = cooling_current_value;
}
