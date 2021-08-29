#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Aux_B {
public:
    Reg_Group_Aux_B() = default;
    Reg_Group_Aux_B(uint16_t g4, uint16_t g5, uint16_t g6) :
        gpio1_voltage(g4),
        gpio2_voltage(g5),
        gpoi3_voltage(g6) { };
    uint16_t get_gpio4_voltage() { return gpio4_voltage; };
    uint16_t get_gpio5_voltage() { return gpio5_voltage; };
    uint16_t get_gpio6_voltage() { return gpio6_voltage; };

    uint8_t* buf() {
        return new uint8_t[6]{gpio4_voltage & 0xFF, (gpio4_voltage & 0xFF00) >> 8, gpio5_voltage & 0xFF, (gpio5_voltage & 0xFF00) >> 8, gpio6_voltage & 0xFF, (gpio6_voltage & 0xFF00) >> 8, };
    };

private:
    uint16_t gpio4_voltage;
    uint16_t gpio5_voltage;
    uint16_t gpio6_voltage;
};
