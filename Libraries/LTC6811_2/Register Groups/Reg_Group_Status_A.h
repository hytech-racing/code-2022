#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Status_A {
public:
    Reg_Group_Status_A() = default;
    Reg_Group_Status_A(uint16_t sc, uint16_t itmp, uint16_t va) :
        sum_of_all_cells(sc),
        internal_die_temp(itmp),
        analog_supply_voltage(va) { };
    uint16_t get_sum_of_all_cells() { return sum_of_all_cells; };
    uint16_t get_internal_die_temp() { return internal_die_temp; };
    uint16_t get_analog_supply_voltage() { return analog_supply_voltage; };

    uint8_t* buf() {
        return new uint8_t[6]{sum_of_all_cells & 0xFF, (sum_of_all_cells & 0xFF00) >> 8, internal_die_temp & 0xFF, (internal_die_temp & 0xFF00) >> 8, analog_supply_voltage & 0xFF, (analog_supply_voltage & 0xFF00) >> 8, };
    };
    
private:
    uint16_t sum_of_all_cells;
    uint16_t internal_die_temp;
    uint16_t analog_supply_voltage;
};