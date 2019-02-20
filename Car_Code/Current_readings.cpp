#include "HyTech_CAN.h"

//Constructors

Current_readings::Current_readings(){
    message = {};
}

Current_readings::Current_readings(uint8_t buf[8]) {
    load(buf);
}

Current_readings::Current_readings(short ecu_current_value, short cooling_current_value) {
    set_ecu_current_value(ecu_current_value);
    set_cooling_current_value(cooling_current_value);
}

//Load from buffer & write to variable instance

void Current_readings::load(uint8_t buf[8]) {
    message = {};

    memcpy(&(message.ecu_current_value), &buf[0], sizeof(short));
    memcpy(&(message.cooling_current_value), &buf[2], sizeof(short));
}

//Write to buffer

void Current_readings::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.ecu_current_value), sizeof(short));
    memcpy(&buf[2], &(message.cooling_current_value), sizeof(short));
}

//Get Functions

short Current_readings::get_ecu_current_value() {
    return message.ecu_current_value;
}

short Current_readings::get_cooling_current_value() {
    return message.cooling_current_value;
}

// Set functions

void Current_readings::set_ecu_current_value(short ecu_current_value) {
    message.ecu_current_value = ecu_current_value;
}

void Current_readings::set_cooling_current_value(short cooling_current_value) {
    message.cooling_current_value = cooling_current_value;
}
