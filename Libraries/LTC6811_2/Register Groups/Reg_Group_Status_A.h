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
    Reg_Group_Status_A(uint8_t* byte_arr) {
        sum_of_all_cells = (byte_arr[1] << 8) + byte_arr[0];
        internal_die_temp = (byte_arr[3] << 8) + byte_arr[2];
        analog_supply_voltage = (byte_arr[5] << 8) + byte_arr[4];
    };
    uint16_t get_sum_of_all_cells() { return sum_of_all_cells; };
    uint16_t get_internal_die_temp() { return internal_die_temp; };
    uint16_t get_analog_supply_voltage() { return analog_supply_voltage; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };
    
private:
    uint16_t sum_of_all_cells;
    uint16_t internal_die_temp;
    uint16_t analog_supply_voltage;
};