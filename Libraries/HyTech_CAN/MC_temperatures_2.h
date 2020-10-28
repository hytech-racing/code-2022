/*
 * MC_temperatures_2.cpp - CAN message parser: RMS Motor Controller temperatures 2 message
 * Created by Nathan Cheek, November 22, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_temperatures_2 {
public:
    MC_temperatures_2() = default;
    MC_temperatures_2(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_control_board_temperature() { return control_board_temperature; }
    inline int16_t get_rtd_1_temperature() { return rtd_1_temperature; }
    inline int16_t get_rtd_2_temperature() { return rtd_2_temperature; }
    inline int16_t get_rtd_3_temperature() { return rtd_3_temperature; }
private:
    int16_t control_board_temperature;
    int16_t rtd_1_temperature;
    int16_t rtd_2_temperature;
    int16_t rtd_3_temperature;
} CAN_message_mc_temperatures_2_t;

#pragma pack(pop)