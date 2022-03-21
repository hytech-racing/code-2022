#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Cell_A {
public:
    Reg_Group_Cell_A() = default;
    Reg_Group_Cell_A(uint16_t c1, uint16_t c2, uint16_t c3) :
        cell1_voltage(c1),
        cell2_voltage(c2),
        cell3_voltage(c3) { };
    Reg_Group_Cell_A(uint8_t* byte_arr) {
        cell1_voltage = (byte_arr[1] << 8) + byte_arr[0];
        cell2_voltage = (byte_arr[3] << 8) + byte_arr[2];
        cell3_voltage = (byte_arr[5] << 8) + byte_arr[4];
    };
    uint16_t get_cell1_voltage() { return cell1_voltage; };
    uint16_t get_cell2_voltage() { return cell2_voltage; };
    uint16_t get_cell3_voltage() { return cell3_voltage; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };
private:
    uint16_t cell1_voltage;
    uint16_t cell2_voltage;
    uint16_t cell3_voltage;
};