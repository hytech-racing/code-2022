#pragma once

#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// @Parseclass
class MCU_analog_readings {
public:
    MCU_analog_readings() = default;
    MCU_analog_readings(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[])  { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) const { memcpy(buf, this, sizeof(*this)); }

    // Getters
    inline uint16_t get_ecu_current()         const { return ecu_current; }
    inline uint16_t get_cooling_current()     const { return cooling_current; }
    inline int16_t  get_temperature()         const { return temperature; }
    inline uint16_t get_glv_battery_voltage() const { return glv_battery_voltage; }

    // Setters
    inline void set_ecu_current(const uint16_t ecu_current_)                { this->ecu_current = ecu_current; }
    inline void set_cooling_current(const uint16_t cooling_current_)        { this->cooling_current = cooling_current; }
    inline void set_temperature(const int16_t temperature)                  { this->temperature = temperature; }
    inline void set_glv_battery_voltage(const uint16_t glv_battery_voltage) { this->glv_battery_voltage = glv_battery_voltage; }

private:
    // @Parse @Unit(A) @Scale(5000)
    uint16_t ecu_current;
	// @Parse @Unit(A) @Scale(5000)
    uint16_t cooling_current;
    // @Parse @Unit(C)
    int16_t temperature;
    // @Parse @Unit(V) @Scale(2500)
    uint16_t glv_battery_voltage;
};

#pragma pack(pop)