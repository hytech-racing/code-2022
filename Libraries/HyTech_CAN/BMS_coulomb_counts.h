#pragma once

#include <string.h>
#include <stdint.h>
#include "Arduino.h"

#pragma pack(push,1)

// @Parseclass @Prefix(BMS)
class BMS_coulomb_counts {
public:
    BMS_coulomb_counts() = default;
    BMS_coulomb_counts(uint8_t buf[]) { load(buf); }
    BMS_coulomb_counts(uint32_t total_charge, uint32_t total_discharge) {
        set_total_charge(total_charge);
        set_total_discharge(total_discharge);
    }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline uint32_t get_total_charge()      const { return total_charge; }
    inline uint32_t get_total_discharge()   const { return total_discharge; }

    inline void set_total_charge(uint32_t total_charge)         { this->total_charge = total_charge; }
    inline void set_total_discharge(uint32_t total_discharge)   { this->total_discharge = total_discharge; }

    void print() {
        Serial.println("\n\nBMS COULOMB COUNTS");
        Serial.println(    "------------------");
        Serial.print("TOTAL CHARGE:    ");  Serial.println(total_charge / 10000., 4);
        Serial.print("TOTAL DISCHARGE: ");  Serial.println(total_discharge / 10000., 4);
    }

private:
    uint32_t total_charge;      // @Parse @Scale(10000) @Unit(C)
    uint32_t total_discharge;   // @Parse @Scale(10000) @Unit(C)
};

#pragma pack(pop)