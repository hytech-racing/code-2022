#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass @Prefix(MC)
class MC_digital_input_status {
public:
    MC_digital_input_status() = default;
    MC_digital_input_status(uint8_t buf[]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline bool get_digital_input_1() const { return digital_input_1; }
    inline bool get_digital_input_2() const { return digital_input_2; }
    inline bool get_digital_input_3() const { return digital_input_3; }
    inline bool get_digital_input_4() const { return digital_input_4; }
    inline bool get_digital_input_5() const { return digital_input_5; }
    inline bool get_digital_input_6() const { return digital_input_6; }
    inline bool get_digital_input_7() const { return digital_input_7; }
    inline bool get_digital_input_8() const { return digital_input_8; }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nMC DIGITAL INPUT STATUS");
        Serial.println(    "-----------------------");
        Serial.print("DIGITAL INPUT 1: ");  Serial.println(digital_input_1);
        Serial.print("DIGITAL INPUT 2: ");  Serial.println(digital_input_2);
        Serial.print("DIGITAL INPUT 3: ");  Serial.println(digital_input_3);
        Serial.print("DIGITAL INPUT 4: ");  Serial.println(digital_input_4);
        Serial.print("DIGITAL INPUT 5: ");  Serial.println(digital_input_5);
        Serial.print("DIGITAL INPUT 6: ");  Serial.println(digital_input_6);
        Serial.print("DIGITAL INPUT 7: ");  Serial.println(digital_input_7);
        Serial.print("DIGITAL INPUT 8: ");  Serial.println(digital_input_8);
    }
#endif

private:
    bool digital_input_1; // @Parse
    bool digital_input_2; // @Parse
    bool digital_input_3; // @Parse
    bool digital_input_4; // @Parse
    bool digital_input_5; // @Parse
    bool digital_input_6; // @Parse
    bool digital_input_7; // @Parse
    bool digital_input_8; // @Parse
};

#pragma pack(pop)
