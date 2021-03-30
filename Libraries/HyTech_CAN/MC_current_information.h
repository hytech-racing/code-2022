#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class MC_current_information {
public:
    MC_current_information() = default;
    MC_current_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_phase_a_current()    const { return phase_a_current; }
    inline int16_t get_phase_b_current()    const { return phase_b_current; }
    inline int16_t get_phase_c_current()    const { return phase_c_current; }
    inline int16_t get_dc_bus_current()     const { return dc_bus_current; }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nMC CURRENT INFORMATION");
        Serial.println(    "----------------------");
        Serial.print("PHASE A CURRENT: ");  Serial.println(phase_a_current / 10.0, 1);
        Serial.print("PHASE B CURRENT: ");  Serial.println(phase_b_current / 10.0, 1);
        Serial.print("PHASE C CURRENT: ");  Serial.println(phase_c_current / 10.0, 1);
        Serial.print("DC BUS CURRENT:  ");  Serial.println(dc_bus_current / 10.0, 1);
    }
#endif

private:
    int16_t phase_a_current; // @Parse @Scale(10) @Unit(A)
    int16_t phase_b_current; // @Parse @Scale(10) @Unit(A)
    int16_t phase_c_current; // @Parse @Scale(10) @Unit(A)
    int16_t dc_bus_current;  // @Parse @Scale(10) @Unit(A)
};

#pragma pack(pop)