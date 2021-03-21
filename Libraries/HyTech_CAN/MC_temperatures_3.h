#pragma once
#include <string.h>
#include <stdint.h>
#include "Arduino.h"

#pragma pack(push,1)

// @Parseclass
class MC_temperatures_3 {
public:
    MC_temperatures_3() = default;
    MC_temperatures_3(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_rtd_4_temperature()  const { return rtd_4_temperature; }
    inline int16_t get_rtd_5_temperature()  const { return rtd_5_temperature; }
    inline int16_t get_motor_temperature()  const { return motor_temperature; }
    inline int16_t get_torque_shudder()     const { return torque_shudder; }

    void print() {
        Serial.println("\n\nMC TEMPERATURES 3");
        Serial.println(    "-----------------");
        Serial.print("RTD 4 TEMP:     ");   Serial.println(rtd_4_temperature);
        Serial.print("RTD 5 TEMP:     ");   Serial.println(rtd_5_temperature);
        Serial.print("MOTOR TEMP:     ");   Serial.println(motor_temperature / 10.0, 1);
        Serial.print("TORQUE SHUDDER: ");   Serial.println(torque_shudder / 10.0, 1);
    }

private:
    int16_t rtd_4_temperature;  // @Parse @Scale(10) @Unit(C)
    int16_t rtd_5_temperature;  // @Parse @Scale(10) @Unit(C)
    int16_t motor_temperature;  // @Parse @Scale(10) @Unit(C)
    int16_t torque_shudder;     // @Parse @Scale(10) @Unit(N-m)
};

#pragma pack(pop)