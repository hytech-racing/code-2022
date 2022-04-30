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
        gpio4_voltage(g4),
        gpio5_voltage(g5),
        gpio6_voltage(g6) { };
    Reg_Group_Aux_B(uint8_t* byte_arr) {
        gpio4_voltage = (byte_arr[1] << 8) + byte_arr[0];
        gpio5_voltage = (byte_arr[3] << 8) + byte_arr[2];
        gpio6_voltage = (byte_arr[5] << 8) + byte_arr[4];
    };
    uint16_t get_gpio4_voltage() { return gpio4_voltage; };
    uint16_t get_gpio5_voltage() { return gpio5_voltage; };
    uint16_t get_gpio6_voltage() { return gpio6_voltage; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };

private:
    uint16_t gpio4_voltage;
    uint16_t gpio5_voltage;
    uint16_t gpio6_voltage;
};
