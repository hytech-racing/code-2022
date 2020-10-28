/*
 * MC_temperatures_3.cpp - CAN message parser: RMS Motor Controller temperatures 3 message
 * Created by Nathan Cheek, November 22, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_temperatures_3 {
public:
    MC_temperatures_3() = default;
    MC_temperatures_3(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_rtd_4_temperature() { return rtd_4_temperature; }
    inline int16_t get_rtd_5_temperature() { return rtd_5_temperature; }
    inline int16_t get_motor_temperature() { return motor_temperature; }
    inline int16_t get_torque_shudder() { return torque_shudder; }
private:
    int16_t rtd_4_temperature;
    int16_t rtd_5_temperature;
    int16_t motor_temperature;
    int16_t torque_shudder;
};

#pragma pack(pop)