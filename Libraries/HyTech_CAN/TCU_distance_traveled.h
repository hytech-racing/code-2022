#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class TCU_distance_traveled {
public:
    TCU_distance_traveled() = default;
    TCU_distance_traveled(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_distance_traveled()  const { return distance_traveled; }

    inline void set_distance_traveled(uint16_t value) { this->distance_traveled = distance_traveled; }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nTCU DISTANCE TRAVELED");
        Serial.println(    "---------------------");
        Serial.print("DISTANCE TRAVELED:   ");  Serial.println(distance_traveled);
    }
#endif

private:
    int16_t distance_traveled; // @Parse @Scale(100) @Unit(m)
};

#pragma pack(pop)