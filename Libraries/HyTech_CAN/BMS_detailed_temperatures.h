/*
 * BMS_detailed_temperatures.cpp - CAN message parser: Battery Management System detailed temperatures message
 * Created by Nathan Cheek, December 17, 2017.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class BMS_detailed_temperatures {
public:
    BMS_detailed_temperatures() = default;
    BMS_detailed_temperatures(uint8_t buf[]) { load(buf); }
    BMS_detailed_temperatures(uint8_t ic_id, int16_t temperature_0, int16_t temperature_1, int16_t temperature_2) {
        set_ic_id(ic_id);
        set_temperature_0(temperature_0);
        set_temperature_1(temperature_1);
        set_temperature_2(temperature_2);
    }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline uint8_t get_ic_id()          { return ic_id; }
    inline int16_t get_temperature_0()  { return temperature_0; }
    inline int16_t get_temperature_1()  { return temperature_1; }
    inline int16_t get_temperature_2()  { return temperature_2; }
    int16_t get_temperature(uint8_t temperature_id) {
        switch (temperature_id) {
            case 0: return temperature_0;
            case 1: return temperature_1;
            case 2: return temperature_2;
        }
        return 0;
    }

    inline void set_ic_id(uint8_t ic_id) { this->ic_id = ic_id; }
    inline void set_temperature_0(int16_t temperature_0) { this->temperature_0 = temperature_0; }
    inline void set_temperature_1(int16_t temperature_1) { this->temperature_1 = temperature_1; }
    inline void set_temperature_2(int16_t temperature_2) { this->temperature_2 = temperature_2; }
    void set_temperature(uint8_t temperature_id, int16_t temperature) {
        switch (temperature_id) {
            case 0: this->temperature_0 = temperature_0; return;
            case 1: this->temperature_1 = temperature_1; return;
            case 2: this->temperature_2 = temperature_2; return;
        }
    }

private:
    uint8_t ic_id;
    int16_t temperature_0;
    int16_t temperature_1;
    int16_t temperature_2;
};

#pragma pack(pop)