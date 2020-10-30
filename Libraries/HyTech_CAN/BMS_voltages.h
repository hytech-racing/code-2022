/*
 * BMS_voltages.cpp - CAN message parser: Battery Management System voltages message
 * Created by Shrivathsav Seshan, January 10, 2017.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class BMS_voltages {
public:
    BMS_voltages() = default;
    BMS_voltages(uint8_t buf[]) { load(buf); }
    BMS_voltages(uint16_t average_voltage, uint16_t low_voltage, uint16_t high_voltage, uint16_t total_voltage) {
        set_average(average_voltage);
        set_low(low_voltage);
        set_high(high_voltage);
        set_total(total_voltage);
    }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_average()   const { return average_voltage; }
    inline uint16_t get_low()       const { return low_voltage; }
    inline uint16_t get_high()      const { return high_voltage; }
    inline uint16_t get_total()     const { return total_voltage; }

    inline void set_average(uint16_t average_voltage) { this->average_voltage = average_voltage; }
    inline void set_low(uint16_t low_voltage)         { this->low_voltage     = low_voltage; }
    inline void set_high(uint16_t high_voltage)       { this->high_voltage    = high_voltage; }
    inline void set_total(uint16_t total_voltage)     { this->total_voltage   = total_voltage; }

private:
    uint16_t average_voltage;
    uint16_t low_voltage;
    uint16_t high_voltage;
    uint16_t total_voltage;
};

#pragma pack(pop)