#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Status_B {
public:
    Reg_Group_Status_B() = default;
    Reg_Group_Status_B(uint16_t vd, uint8_t byte2_, uint8_t byte3_, uint8_t byte4_, uint8_t byte5_) :
        digital_supply_voltage(vd),
        byte2(byte2_),
        byte3(byte3_),
        byte4(byte4_),
        byte5(byte5_) { 
    };
    Reg_Group_Status_B(uint8_t *byte_arr) {
        digital_supply_voltage = (byte_arr[1] << 8) + byte_arr[0];
        byte2 = byte_arr[2];
        byte3 = byte_arr[3];
        byte4 = byte_arr[4];
        byte5 = byte_arr[5];
    };


    uint16_t get_digital_supply_voltage() { return digital_supply_voltage; };

    //make a better way of doing this that makes more sense please
    uint16_t get_cell_overvoltage_flags() { 
        return ((byte2 & 0b10) >> 1) + 
            ((byte2 & 0b1000) >> 2) + 
            ((byte2 & 0b100000) >> 3) + 
            ((byte2 & 0b10000000) >> 4) +
            ((byte3 & 0b10) << 3) + 
            ((byte3 & 0b1000) << 2) + 
            ((byte3 & 0b100000) << 1) + 
            (byte3 & 0b10000000) +
            ((byte3 & 0b10) << 7) + 
            ((byte3 & 0b1000) << 6) + 
            ((byte3 & 0b100000) << 5) + 
            ((byte3 & 0b10000000) << 4);
    };
    uint16_t get_cell_undervoltage_flags() { 
        return (byte2 & 0b1) + 
            ((byte2 & 0b100) >> 1) + 
            ((byte2 & 0b10000) >> 2) + 
            ((byte2 & 0b1000000) >> 3) +
            ((byte3 & 0b1) << 4) + 
            ((byte3 & 0b100) << 3) + 
            ((byte3 & 0b10000) << 2) + 
            ((byte3 & 0b1000000) << 1) +
            ((byte4 & 0b1) << 8) + 
            ((byte4 & 0b100) << 7) + 
            ((byte4 & 0b10000) << 6) + 
            ((byte4 & 0b1000000) << 5); 
    };
    uint8_t get_revision_code() { return (byte5 & 0xF0) >> 4; };
    bool get_mux_self_test_result() { return (byte5 & 0b10) > 0; };
    bool get_thermal_shutdown_status() { return (byte5 & 0b1) > 0; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };

private:
    uint16_t digital_supply_voltage;

    uint8_t byte2, byte3, byte4, byte5;
};