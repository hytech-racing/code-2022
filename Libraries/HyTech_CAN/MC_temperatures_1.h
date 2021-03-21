#pragma once
#include <string.h>
#include <stdint.h>
#include "Arduino.h"

#pragma pack(push,1)

// @Parseclass
class MC_temperatures_1 {
public:
    MC_temperatures_1() = default;
    MC_temperatures_1(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_module_a_temperature()           const { return module_a_temperature; }
    inline int16_t get_module_b_temperature()           const { return module_b_temperature; }
    inline int16_t get_module_c_temperature()           const { return module_c_temperature; }
    inline int16_t get_gate_driver_board_temperature()  const { return gate_driver_board_temperature; }

    void print() {
        Serial.println("\n\nMC TEMPERATURES 1");
        Serial.println(    "-----------------");
        Serial.print("MODULE A TEMP:          ");   Serial.println(module_a_temperature / 10.0, 1);
        Serial.print("MODULE B TEMP:          ");   Serial.println(module_b_temperature / 10.0, 1);
        Serial.print("MODULE C TEMP:          ");   Serial.println(module_c_temperature / 10.0, 1);
        Serial.print("GATE DRIVER BOARD TEMP: ");   Serial.println(gate_driver_board_temperature / 10.0, 1);
    }

private:
    int16_t module_a_temperature;           // @Parse @Scale(10) @Unit(C)
    int16_t module_b_temperature;           // @Parse @Scale(10) @Unit(C)
    int16_t module_c_temperature;           // @Parse @Scale(10) @Unit(C)
    int16_t gate_driver_board_temperature;  // @Parse @Scale(10) @Unit(C)
};

#pragma pack(pop)