#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

class MC_current_information {
public:
    MC_current_information() = default;
    MC_current_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_phase_a_current()    const { return phase_a_current; }
    inline int16_t get_phase_b_current()    const { return phase_b_current; }
    inline int16_t get_phase_c_current()    const { return phase_c_current; }
    inline int16_t get_dc_bus_current()     const { return dc_bus_current; }
private:
    int16_t phase_a_current;
    int16_t phase_b_current;
    int16_t phase_c_current;
    int16_t dc_bus_current;
};

#pragma pack(pop)