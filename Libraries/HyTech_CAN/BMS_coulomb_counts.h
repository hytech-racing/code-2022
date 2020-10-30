/*
 *BMS_coulomb_counts - CAN message parser: Battery Management System coulomb count message
 * Created by Shaan Dhawan, March 26, 2019.
 */

#pragma once

#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class BMS_coulomb_counts {
public:
    BMS_coulomb_counts() = default;
    BMS_coulomb_counts(uint8_t buf[]) { load(buf); }
    BMS_coulomb_counts(uint32_t total_charge, uint32_t total_discharge) {
        set_total_charge(total_charge);
        set_total_discharge(total_discharge);
    }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline uint32_t get_total_charge()      const { return total_charge; }
    inline uint32_t get_total_discharge()   const { return total_discharge; }

    inline void set_total_charge(uint32_t total_charge)         { this->total_charge = total_charge; }
    inline void set_total_discharge(uint32_t total_discharge)   { this->total_discharge = total_discharge; }

private:
    uint32_t total_charge;
    uint32_t total_discharge;
};

#pragma pack(pop)