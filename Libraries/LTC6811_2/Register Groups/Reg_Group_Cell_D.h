#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Cell_D {
public:
    Reg_Group_Cell_D() = default;
    Reg_Group_Cell_D(uint16_t c10, uint16_t c11, uint16_t c12) :
        cell10_voltage(c10),
        cell11_voltage(c11),
        cell12_voltage(c12) { };
    Reg_Group_Cell_D(uint8_t* byte_arr) {
        cell10_voltage = (byte_arr[1] << 8) + byte_arr[0];
        cell11_voltage = (byte_arr[3] << 8) + byte_arr[2];
        cell12_voltage = (byte_arr[5] << 8) + byte_arr[4];
    };
    uint16_t get_cell10_voltage() { return cell10_voltage; };
    uint16_t get_cell11_voltage() { return cell11_voltage; };
    uint16_t get_cell12_voltage() { return cell12_voltage; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };

private:
    uint16_t cell10_voltage;
    uint16_t cell11_voltage;
    uint16_t cell12_voltage;
};