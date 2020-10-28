/*
 * MC_analog_input_voltages.cpp - CAN message parser: RMS Motor Controller analog input voltages message
 * Created by Ryan Gallaway, December 1, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_analog_input_voltages {
public:
    MC_analog_input_voltages() = default;
    MC_analog_input_voltages(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_analog_input_1() { return analog_input_1; }
    inline int16_t get_analog_input_2() { return analog_input_2; }
    inline int16_t get_analog_input_3() { return analog_input_3; }
    inline int16_t get_analog_input_4() { return analog_input_4; }

private:
    int16_t analog_input_1;
    int16_t analog_input_2;
    int16_t analog_input_3;
    int16_t analog_input_4;
};

#pragma pack(pop)