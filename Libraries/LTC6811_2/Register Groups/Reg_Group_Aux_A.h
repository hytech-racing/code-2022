#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Aux_A {
public:
    Reg_Group_Aux_A() = default;
    Reg_Group_Aux_A(uint16_t g1, uint16_t g2, uint16_t g3) :
        gpio1_voltage(g1),
        gpio2_voltage(g2),
        gpio3_voltage(g3) {
        };
    Reg_Group_Aux_A(uint8_t* byte_arr) {
        gpio1_voltage = (byte_arr[1] << 8) + byte_arr[0];
        gpio2_voltage = (byte_arr[3] << 8) + byte_arr[2];
        gpio3_voltage = (byte_arr[5] << 8) + byte_arr[4];
    };
    uint16_t get_gpio1_voltage() { return gpio1_voltage; };
    uint16_t get_gpio2_voltage() { return gpio2_voltage; };
    uint16_t get_gpio3_voltage() { return gpio3_voltage; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };

private:
    uint16_t gpio1_voltage;
    uint16_t gpio2_voltage;
    uint16_t gpio3_voltage;
};