#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_COMM {
public:
    Reg_Group_COMM() = default;
    Reg_Group_COMM(uint8_t d0, 
                    uint8_t d1, 
                    uint8_t d2, 
                    uint8_t icom0, 
                    uint8_t icom1, 
                    uint8_t icom2, 
                    uint8_t fcom0, 
                    uint8_t fcom1,
                    uint8_t fcom2) {
        byte0 = (icom0 << 4) + ((d0 & 0xF0) >> 4);
        byte1 = ((d0 & 0x0F) << 4) + fcom0;
        byte2 = (icom1 << 4) + ((d1 & 0xF0) >> 4);
        byte3 = ((d1 & 0x0F) << 4) + fcom1;
        byte4 = (icom2 << 4) + ((d2 & 0xF0) >> 4);
        byte5 = ((d2 & 0x0F) << 4) + fcom2;
    };
    Reg_Group_COMM(uint8_t* byte_arr) : 
        byte0(byte_arr[0]), 
        byte1(byte_arr[1]), 
        byte2(byte_arr[2]), 
        byte3(byte_arr[3]), 
        byte4(byte_arr[4]), 
        byte5(byte_arr[5]) { };

    uint8_t get_data_byte0() { return ((byte1 & 0xF0) >> 4) + ((byte0 & 0x0F) << 4); };
    uint8_t get_data_byte1() { return ((byte3 & 0xF0) >> 4) + ((byte2 & 0x0F) << 4); };
    uint8_t get_data_byte2() { return ((byte5 & 0xF0) >> 4) + ((byte4 & 0x0F) << 4); };
    uint8_t initial_comm_control0() { return (byte0 & 0xF0) >> 4; };
    uint8_t initial_comm_control1() { return (byte2 & 0xF0) >> 4; };
    uint8_t initial_comm_control2() { return (byte4 & 0xF0) >> 4; };
    uint8_t final_comm_control0() { return byte1 & 0x0F; };
    uint8_t final_comm_control1() { return byte3 & 0x0F; };
    uint8_t final_comm_control2() { return byte5 & 0x0F; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };


private:
    uint8_t byte0, byte1, byte2, byte3, byte4, byte5;
};