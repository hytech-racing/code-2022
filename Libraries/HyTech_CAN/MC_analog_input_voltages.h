
#pragma once
#include <string.h>
#include <stdint.h>
#include "Arduino.h"

#pragma pack(push,1)

// @Parseclass @ID(ID_MC_ANALOG_INPUTS_VOLTAGES) @Prefix(MC)
class MC_analog_input_voltages {
public:
    MC_analog_input_voltages() = default;
    MC_analog_input_voltages(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_analog_input_1() const { return analog_input_1; }
    inline int16_t get_analog_input_2() const { return analog_input_2; }
    inline int16_t get_analog_input_3() const { return analog_input_3; }
    inline int16_t get_analog_input_4() const { return analog_input_4; }

    void print() {
        Serial.println("\n\nMC ANALOG INPUT VOLTAGES");
        Serial.println(    "------------------------");
        Serial.print("ANALOG INPUT 1: ");  Serial.println(analog_input_1);
        Serial.print("ANALOG INPUT 2: ");  Serial.println(analog_input_2);
        Serial.print("ANALOG INPUT 3: ");  Serial.println(analog_input_3);
        Serial.print("ANALOG INPUT 4: ");  Serial.println(analog_input_4);
    }

private:
    int16_t analog_input_1; // @Parse
    int16_t analog_input_2; // @Parse
    int16_t analog_input_3; // @Parse
    int16_t analog_input_4; // @Parse
};

#pragma pack(pop)