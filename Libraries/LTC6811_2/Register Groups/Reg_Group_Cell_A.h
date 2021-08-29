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
    uint16_t get_cell1_voltage() { return cell1_voltage; };
    uint16_t get_cell2_voltage() { return cell2_voltage; };
    uint16_t get_cell3_voltage() { return cell3_voltage; };

    uint8_t* buf() {
        return new uint8_t[6]{cell1_voltage & 0xFF, (cell1_voltage & 0xFF00) >> 8, cell2_voltage & 0xFF, (cell2_voltage & 0xFF00) >> 8, cell3_voltage & 0xFF, (cell3_voltage & 0xFF00) >> 8, };
    };
private:
    uint16_t cell1_voltage;
    uint16_t cell2_voltage;
    uint16_t cell3_voltage;
};