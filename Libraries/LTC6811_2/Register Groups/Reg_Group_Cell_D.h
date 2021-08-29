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
    uint16_t get_cell10_voltage() { return cell10_voltage; };
    uint16_t get_cell11_voltage() { return cell11_voltage; };
    uint16_t get_cell12_voltage() { return cell12_voltage; };

    uint8_t* buf() {
        return new uint8_t[6]{cell10_voltage & 0xFF, (cell10_voltage & 0xFF00) >> 8, cell11_voltage & 0xFF, (cell11_voltage & 0xFF00) >> 8, cell12_voltage & 0xFF, (cell12_voltage & 0xFF00) >> 8, };
    };

private:
    uint16_t cell10_voltage;
    uint16_t cell11_voltage;
    uint16_t cell12_voltage;
};