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
        cell1_voltage(c7),
        cell2_voltage(c8),
        cell3_voltage(c9) { };
    uint16_t get_cell7_voltage() { return cell7_voltage; };
    uint16_t get_cell8_voltage() { return cell8_voltage; };
    uint16_t get_cell9_voltage() { return cell9_voltage; };

    uint8_t* buf() {
        return new uint8_t[6]{cell7_voltage & 0xFF, (cell7_voltage & 0xFF00) >> 8, cell8_voltage & 0xFF, (cell8_voltage & 0xFF00) >> 8, cell9_voltage & 0xFF, (cell9_voltage & 0xFF00) >> 8, };
    };

private:
    uint16_t cell7_voltage;
    uint16_t cell8_voltage;
    uint16_t cell9_voltage;
};
