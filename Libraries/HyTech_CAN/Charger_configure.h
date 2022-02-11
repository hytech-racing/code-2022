#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class Charger_configure {
public:
    Charger_configure() = default;
    Charger_configure(uint8_t buf[]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])        { memcpy(buf, this, sizeof(*this)); }

    inline uint8_t get_control()      { return control; }
    inline void set_control(uint16_t con) { this->control = con; }
    inline uint16_t get_max_charging_voltage()  { return max_charging_voltage; }
    inline void set_max_charging_voltage(uint16_t max_charging_voltage) { this->max_charging_voltage = max_charging_voltage; }
    inline uint16_t get_max_charging_current()  { return max_charging_current; }
    inline void set_max_charging_current(uint16_t max_charging_current) { this->max_charging_current = max_charging_current; }

private:
    uint16_t max_charging_voltage;
    uint16_t max_charging_current;
    uint8_t control;
};

#pragma pack(pop)