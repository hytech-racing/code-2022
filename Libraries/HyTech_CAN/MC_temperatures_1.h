#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

class MC_temperatures_1 {
public:
    MC_temperatures_1() = default;
    MC_temperatures_1(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_module_a_temperature()           const { return module_a_temperature; }
    inline int16_t get_module_b_temperature()           const { return module_b_temperature; }
    inline int16_t get_module_c_temperature()           const { return module_c_temperature; }
    inline int16_t get_gate_driver_board_temperature()  const { return gate_driver_board_temperature; }
private:
    int16_t module_a_temperature;
    int16_t module_b_temperature;
    int16_t module_c_temperature;
    int16_t gate_driver_board_temperature;
};

#pragma pack(pop)