/*
 * MC_firmware_information.cpp - CAN message parser: RMS Motor Controller firmware information message
 * Created by Nathan Cheek, November 23, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_firmware_information {
public:
    MC_firmware_information() = default;
    MC_firmware_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_eeprom_version_project_code() { return eeprom_version_project_code; }
    inline uint16_t get_software_version() { return software_version; }
    inline uint16_t get_date_code_mmdd() { return date_code_mmdd; }
    inline uint16_t get_date_code_yyyy() { return date_code_yyyy; }

private:
    uint16_t eeprom_version_project_code;
    uint16_t software_version;
    uint16_t date_code_mmdd;
    uint16_t date_code_yyyy;
};

#pragma pack(pop)