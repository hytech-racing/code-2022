#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Cell_B {
public:
    Reg_Group_Cell_B() = default;
    Reg_Group_Cell_B(uint16_t c4, uint16_t c5, uint16_t c6) :
        cell4_voltage(c4),
        cell5_voltage(c5),
        cell6_voltage(c6) { };
    Reg_Group_Cell_B(uint8_t* byte_arr) {
        cell4_voltage = (byte_arr[1] << 8) + byte_arr[0];
        cell5_voltage = (byte_arr[3] << 8) + byte_arr[2];
        cell6_voltage = (byte_arr[5] << 8) + byte_arr[4];
    };
    uint16_t get_cell4_voltage() { return cell4_voltage; };
    uint16_t get_cell5_voltage() { return cell5_voltage; };
    uint16_t get_cell6_voltage() { return cell6_voltage; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };
private:
    uint16_t cell4_voltage;
    uint16_t cell5_voltage;
    uint16_t cell6_voltage;
};