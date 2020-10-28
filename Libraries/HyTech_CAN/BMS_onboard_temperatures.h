/*
 * BMS_onboard_temperatures.cpp - CAN message parser: Battery Management System PCB onboard temperatures message
 * Created by Yvone Yeh, February 8, 2018.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class BMS_onboard_temperatures {
public:
    BMS_onboard_temperatures() = default;
    BMS_onboard_temperatures(uint8_t buf[]) { load(buf); }
    BMS_onboard_temperatures(int16_t average_temperature, int16_t low_temperature, int16_t high_temperature) {
        set_average_temperature(average_temperature);
        set_low_temperature(low_temperature);
        set_high_temperature(high_temperature);
    }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_average_temperature() { return average_temperature; }
    inline int16_t get_low_temperature() { return low_temperature; }
    inline int16_t get_high_temperature() { return high_temperature; }

    inline void set_average_temperature(int16_t average_temperature) { this->average_temperature = average_temperature; }
    inline void set_low_temperature(int16_t low_temperature) { this->low_temperature = low_temperature; }
    inline void set_high_temperature(int16_t high_temperature) { this->high_temperature = high_temperature; }

private:
    int16_t average_temperature;
    int16_t low_temperature;
    int16_t high_temperature;
};

#pragma pack(pop)