/*
 * BMS_onboard_detailed_temperatures.cpp - CAN message parser: Battery Management System PCB onboard detailed temperatures message
 * Created by Yvonne Yeh, February 8, 2018.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class BMS_onboard_detailed_temperatures {
public:
    BMS_onboard_detailed_temperatures() = default;
    BMS_onboard_detailed_temperatures(uint8_t buf[]) { load(buf); }
    BMS_onboard_detailed_temperatures(uint8_t ic_id, int16_t temperature_0, int16_t temperature_1) {
        set_ic_id(ic_id); 
        set_temperature_0(temperature_0); 
        set_temperature_1(temperature_1);
    }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline uint8_t get_ic_id() { return ic_id; }
    inline int16_t get_temperature_0() { return temperature_0; }
    inline int16_t get_temperature_1() { return temperature_1; }
    int16_t get_temperature(uint8_t temperature_id) {
        switch (temperature_id) {
            case 0: return temperature_0;
            case 1: return temperature_1;
        }
        return 0;
    }

    inline void set_ic_id(uint8_t ic_id) { this->ic_id = ic_id; }
    inline void set_temperature_0(int16_t temperature_0) { this->temperature_0 = temperature_0; }
    inline void set_temperature_1(int16_t temperature_1) { this->temperature_1 = temperature_1; }
    void set_temperature(uint8_t temperature_id, int16_t temperature) {
        switch (temperature_id) {
            case 0: temperature_0 = temperature; return;
            case 1: temperature_1 = temperature; return;
        }
    }

private:
    uint8_t ic_id;
    int16_t temperature_0;
    int16_t temperature_1;
};

#pragma pack(pop)