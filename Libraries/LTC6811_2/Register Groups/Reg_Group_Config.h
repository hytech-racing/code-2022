#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_Config {
public:
    Reg_Group_Config() = default;
    Reg_Group_Config(uint8_t gpio_, bool refon_, bool adcpot_, uint16_t undervoltage_, uint16_t overvoltage_, uint16_t discharge_, uint8_t dcto_) {
        byte0 = (gpio_ << 3) | (static_cast<int>(refon_) << 2) | static_cast<int>(adcpot_);
        byte1 = (undervoltage_ & 0x0FF);
        byte2 = ((overvoltage_ & 0x00F) << 4) | ((undervoltage_ & 0xF00) >> 8);
        byte3 = ((overvoltage_ & 0xFF0) >> 4);
        byte4 = ((discharge_ & 0x0FF));
        byte5 = ((dcto_ & 0x0F) << 4) | ((discharge_ & 0xF00) >> 8);
    };
    Reg_Group_Config(uint8_t* byte_arr) :
        byte0(byte_arr[0]), 
        byte1(byte_arr[1]), 
        byte2(byte_arr[2]), 
        byte3(byte_arr[3]), 
        byte4(byte_arr[4]), 
        byte5(byte_arr[5]) { };

    uint8_t get_gpio() { return (byte0 & 0b11111000) >> 3; };
    bool get_refon() { return (byte0 & 0b100) > 0; };
    bool get_dten() { return (byte0 & 0b10) > 0; };
    bool get_adcpot() { return (byte0 & 0b1) > 0; };
    uint16_t get_undervoltage() { return ((byte2 & 0x0F) << 8) | (byte1); };
    uint16_t get_overvoltage() { return ((byte2 & 0xF0) >> 4) | (byte3 << 4); };
    uint16_t get_discharge() { return ((byte5 & 0x0F) << 8) | byte4; };
    uint8_t get_dcto() { return (byte5 & 0xF0) >> 4; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };

private:
    uint8_t byte0, byte1, byte2, byte3, byte4, byte5;
};

enum class CELL_DISCHARGE {
    CELL_1 = 1<<0,
    CELL_2 = 1<<1,
    CELL_3 = 1<<2,
    CELL_4 = 1<<3,
    CELL_5 = 1<<4,
    CELL_6 = 1<<5,
    CELL_7 = 1<<6,
    CELL_8 = 1<<7,
    CELL_9 = 1<<8,
    CELL_10 = 1<<9,
    CELL_11 = 1<<10,
    CELL_12 = 1<<11
};

enum class GPIO_CONFIG {
    GPIO1_SET = 1<<0,
    GPIO2_SET = 1<<1,
    GPIO3_SET = 1<<2,
    GPIO4_SET = 1<<3,
    GPIO5_SET = 1<<4,
};