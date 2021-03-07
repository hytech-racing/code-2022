/*
 * MC_firmware_information.cpp - CAN message parser: RMS Motor Controller firmware information message
 * Created by Nathan Cheek, November 23, 2016.
 */

#include "HyTech_CAN.h"

MC_firmware_information::MC_firmware_information() {
    message = {};
}

MC_firmware_information::MC_firmware_information(uint8_t buf[8]) {
    load(buf);
}

void MC_firmware_information::load(uint8_t buf[8]) {
    message = {};
    memcpy(&(message.eeprom_version_project_code), &buf[0], sizeof(uint16_t));
    memcpy(&(message.software_version), &buf[2], sizeof(uint16_t));
    memcpy(&(message.date_code_mmdd), &buf[4], sizeof(uint16_t));
    memcpy(&(message.date_code_yyyy), &buf[6], sizeof(uint16_t));
}

void MC_firmware_information::write(uint8_t buf[8]) {
    memcpy(&buf[0], &(message.eeprom_version_project_code), sizeof(uint16_t));
    memcpy(&buf[2], &(message.software_version), sizeof(uint16_t));
    memcpy(&buf[4], &(message.date_code_mmdd), sizeof(uint16_t));
    memcpy(&buf[6], &(message.date_code_yyyy), sizeof(uint16_t));
}

uint16_t MC_firmware_information::get_eeprom_version_project_code() {
    return message.eeprom_version_project_code;
}

uint16_t MC_firmware_information::get_software_version() {
    return message.software_version;
}

uint16_t MC_firmware_information::get_date_code_mmdd() {
    return message.date_code_mmdd;
}

uint16_t MC_firmware_information::get_date_code_yyyy() {
    return message.date_code_yyyy;
}
