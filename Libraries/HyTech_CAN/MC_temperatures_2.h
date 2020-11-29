#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

class MC_temperatures_2 {
public:
    MC_temperatures_2() = default;
    MC_temperatures_2(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_control_board_temperature()  const { return control_board_temperature; }
    inline int16_t get_rtd_1_temperature()          const { return rtd_1_temperature; }
    inline int16_t get_rtd_2_temperature()          const { return rtd_2_temperature; }
    inline int16_t get_rtd_3_temperature()          const { return rtd_3_temperature; }
private:
    int16_t control_board_temperature;
    int16_t rtd_1_temperature;
    int16_t rtd_2_temperature;
    int16_t rtd_3_temperature;
};

#pragma pack(pop)