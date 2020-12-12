#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

// @Parseclass
class MC_motor_position_information {
public:
    MC_motor_position_information() = default;
    MC_motor_position_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_motor_angle()                    const { return motor_angle; }
    inline int16_t get_motor_speed()                    const { return motor_speed; }
    inline int16_t get_electrical_output_frequency()    const { return electrical_output_frequency; }
    inline int16_t get_delta_resolver_filtered()        const { return delta_resolver_filtered; }
private:
    int16_t motor_angle; // @Parse @Scale(10)
    int16_t motor_speed; // @Parse @Unit(RPM)
    int16_t electrical_output_frequency; // @Parse @Scale(10)
    int16_t delta_resolver_filtered; // @Parse
};

#pragma pack(pop)