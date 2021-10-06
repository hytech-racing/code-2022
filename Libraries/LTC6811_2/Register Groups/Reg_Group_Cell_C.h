#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Cell_C {
public:
    Reg_Group_Cell_C() = default;
    Reg_Group_Cell_C(uint16_t c7, uint16_t c8, uint16_t c9) :
        cell7_voltage(c7),
        cell8_voltage(c8),
        cell9_voltage(c9) { };
    Reg_Group_Cell_C(uint8_t* byte_arr) {
        cell7_voltage = (byte_arr[1] << 8) + byte_arr[0];
        cell8_voltage = (byte_arr[3] << 8) + byte_arr[2];
        cell9_voltage = (byte_arr[5] << 8) + byte_arr[4];
    };
    uint16_t get_cell7_voltage() { return cell7_voltage; };
    uint16_t get_cell8_voltage() { return cell8_voltage; };
    uint16_t get_cell9_voltage() { return cell9_voltage; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };

private:
    uint16_t cell7_voltage;
    uint16_t cell8_voltage;
    uint16_t cell9_voltage;
};
