#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
#include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class EM_measurement {
public:
    EM_measurement() = default;
    EM_measurement(uint8_t buf[]) { load(buf); }

    inline void load(uint8_t buf[])           { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    const { memcpy(buf, this, sizeof(*this)); }

    // Big endian byte ordering
    inline void load_from_emeter(uint8_t buf[]) {
        voltage = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]);
        current = (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | (buf[7]);
    }

    // Big endian byte ordering
    inline void write_for_emeter(uint8_t buf[]) {
        buf[0] = voltage >> 24;
        buf[1] = voltage >> 16  & 0xFF;
        buf[2] = voltage >>  8  & 0xFF;
        buf[3] = voltage        & 0xFF;
        buf[4] = current >> 24;
        buf[5] = current >> 16  & 0xFF;
        buf[6] = current >>  8  & 0xFF;
        buf[7] = current        & 0xFF;
    }

    inline uint8_t get_voltage()        const { return voltage;  }
    inline uint8_t get_current()        const { return current;  }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nEM MEASUREMENT");
		Serial.println(     "---------");
		Serial.print("VOLTAGE: ");      Serial.println(get_voltage() * 1.5258789063E-005 );
		Serial.print("CURRENT: ");      Serial.println(get_current() * 1.5258789063E-005 );
	}
#endif

private:
    uint32_t voltage; //@Parse @Scale(65536) @Unit(V)
    uint32_t current; //@Parse @Scale(65536) @Unit(A)
};

#pragma pack(pop) 