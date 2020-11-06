#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_digital_input_status {
public:
    MC_digital_input_status() = default;
    MC_digital_input_status(uint8_t buf[]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline bool get_digital_input_1() const { return digital_input_1; }
    inline bool get_digital_input_2() const { return digital_input_2; }
    inline bool get_digital_input_3() const { return digital_input_3; }
    inline bool get_digital_input_4() const { return digital_input_4; }
    inline bool get_digital_input_5() const { return digital_input_5; }
    inline bool get_digital_input_6() const { return digital_input_6; }
    inline bool get_digital_input_7() const { return digital_input_7; }
    inline bool get_digital_input_8() const { return digital_input_8; }
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
