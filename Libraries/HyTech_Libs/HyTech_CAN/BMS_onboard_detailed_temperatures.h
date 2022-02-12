#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass @Prefix(IC_{get_ic_id()}) @Indexable(get_ic_id(8))
class BMS_onboard_detailed_temperatures {
public:
    BMS_onboard_detailed_temperatures() = default;
    BMS_onboard_detailed_temperatures(uint8_t buf[]) { load(buf); }
    BMS_onboard_detailed_temperatures(uint8_t ic_id, int16_t temperature_0, int16_t temperature_1) {
        set_ic_id(ic_id); 
        set_temperature_0(temperature_0); 
        set_temperature_1(temperature_1);
    }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline uint8_t get_ic_id()          const { return ic_id; }
    inline int16_t get_temperature_0()  const { return temperature_0; }
    inline int16_t get_temperature_1()  const { return temperature_1; }
    int16_t get_temperature(uint8_t temperature_id) const {
        switch (temperature_id) {
            case 0: return temperature_0;
            case 1: return temperature_1;
        }
        return 0;
    }

    inline void set_ic_id(uint8_t ic_id)                 { this->ic_id = ic_id; }
    inline void set_temperature_0(int16_t temperature_0) { this->temperature_0 = temperature_0; }
    inline void set_temperature_1(int16_t temperature_1) { this->temperature_1 = temperature_1; }
    void set_temperature(uint8_t temperature_id, int16_t temperature) {
        switch (temperature_id) {
            case 0: temperature_0 = temperature; return;
            case 1: temperature_1 = temperature; return;
        }
    }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nBMS ONBOARD DETAILED TEMPERATURES");
        Serial.println(    "---------------------------------");
        Serial.print("IC:            ");    Serial.println((uint32_t) ic_id);
        Serial.print("TEMPERATURE 0: ");    Serial.println(temperature_0 / 100., 2);
        Serial.print("TEMPERATURE 1: ");    Serial.println(temperature_1 / 100., 2);
    }
#endif

private:
    uint8_t ic_id;
    int16_t temperature_0; // @Parse @Scale(100) @Unit(C)
    int16_t temperature_1; // @Parse @Scale(100) @Unit(C)
};

#pragma pack(pop)