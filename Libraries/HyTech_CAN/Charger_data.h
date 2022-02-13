#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class Charger_data {
public:
    Charger_data() = default;
    Charger_data(uint8_t buf[]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }

    inline uint8_t get_output_current() {return output_current;}
    inline uint8_t get_charger_status() {return charger_status;}
    inline uint8_t get_flags() {return flags;}
    inline uint16_t get_output_dc_voltage(){return output_dc_voltage;}
    inline uint16_t get_input_ac_voltage(){return input_ac_voltage;}

private:
    uint16_t output_dc_voltage;
    uint8_t output_current;
    uint8_t charger_status;
    uint8_t flags;
    uint16_t input_ac_voltage;
};

#pragma pack(pop)