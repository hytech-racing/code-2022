#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class GLV_current_readings {
public:
    GLV_current_readings() = default;
    GLV_current_readings(uint8_t buf[8]) { load(buf); }
    GLV_current_readings(uint16_t ecu_current_value, uint16_t cooling_current_value) {
        set_ecu_current_value(ecu_current_value);
        set_cooling_current_value(cooling_current_value);
    }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_ecu_current_value()     const { return ecu_current_value; }
    inline uint16_t get_cooling_current_value() const { return cooling_current_value; }

    inline void set_ecu_current_value(uint16_t ecu_current_value)         { this->ecu_current_value = ecu_current_value; }
    inline void set_cooling_current_value(uint16_t cooling_current_value) { this->cooling_current_value = cooling_current_value; }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nGLV Current");
        Serial.println(    "-----------");
        Serial.print("ECU Current:     ");  Serial.println(ecu_current_value / 100., 2);
        Serial.print("Cooling Current: ");  Serial.println(cooling_current_value / 100., 2);
    }
#endif

private:
    uint16_t ecu_current_value;     // @Parse @Name(ecu_current) @Scale(100) @Unit(A)
	uint16_t cooling_current_value; // @Parse @Name(cooling_current) @Scale(100) @Unit(A)
};


#pragma pack(pop)