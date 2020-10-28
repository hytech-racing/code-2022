/*
 * MC_digital_input_status.cpp - CAN message parser: RMS Motor Controller digital input status message
 * Created by Ryan Gallaway, December 1, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_digital_input_status {
public:
    MC_digital_input_status() = default;
    MC_digital_input_status(uint8_t buf[]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline bool get_digital_input_1() { return digital_input_1; }
    inline bool get_digital_input_2() { return digital_input_2; }
    inline bool get_digital_input_3() { return digital_input_3; }
    inline bool get_digital_input_4() { return digital_input_4; }
    inline bool get_digital_input_5() { return digital_input_5; }
    inline bool get_digital_input_6() { return digital_input_6; }
    inline bool get_digital_input_7() { return digital_input_7; }
    inline bool get_digital_input_8() { return digital_input_8; }
private:
    bool digital_input_1;
    bool digital_input_2;
    bool digital_input_3;
    bool digital_input_4;
    bool digital_input_5;
    bool digital_input_6;
    bool digital_input_7;
    bool digital_input_8;
};

#pragma pack(pop)
