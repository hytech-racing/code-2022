#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif


enum class S_CONTROL {
    S_DRIVE_HIGH = 0,
    S_1_PULSE,
    S_2_PULSES,
    S_3_PULSES,
    S_4_PULSES,
    S_5_PULSES,
    S_6_PULSES,
    S_7_PULSES,
    S_DRIVE_LOW
};

class Reg_Group_S_Ctrl {
public:
    Reg_Group_S_Ctrl() = default;
    Reg_Group_S_Ctrl(S_CONTROL pin1_,
                    S_CONTROL pin2_, 
                    S_CONTROL pin3_,
                    S_CONTROL pin4_,
                    S_CONTROL pin5_,
                    S_CONTROL pin6_,
                    S_CONTROL pin7_,
                    S_CONTROL pin8_,
                    S_CONTROL pin9_,
                    S_CONTROL pin10_,
                    S_CONTROL pin11_,
                    S_CONTROL pin12_) {
        byte0 = ((int)pin2_ << 4) + (int)pin1_;
        byte1 = ((int)pin4_ << 4) + (int)pin3_;
        byte2 = ((int)pin6_ << 4) + (int)pin5_;
        byte3 = ((int)pin8_ << 4) + (int)pin7_;
        byte4 = ((int)pin10_ << 4) + (int)pin9_;
        byte5 = ((int)pin12_ << 4) + (int)pin11_;
    };
    Reg_Group_S_Ctrl(uint8_t* byte_arr) : 
        byte0(byte_arr[0]), 
        byte1(byte_arr[1]), 
        byte2(byte_arr[2]), 
        byte3(byte_arr[3]), 
        byte4(byte_arr[4]), 
        byte5(byte_arr[5]) { };

    S_CONTROL get_pin1() { return (S_CONTROL)(byte0 & 0x0F); };
    S_CONTROL get_pin2() { return (S_CONTROL)((byte0 & 0xF0) >> 4); };
    S_CONTROL get_pin3() { return (S_CONTROL)(byte1 & 0x0F); };
    S_CONTROL get_pin4() { return (S_CONTROL)((byte1 & 0xF0) >> 4); };
    S_CONTROL get_pin5() { return (S_CONTROL)(byte2 & 0x0F); };
    S_CONTROL get_pin6() { return (S_CONTROL)((byte2 & 0xF0) >> 4); };
    S_CONTROL get_pin7() { return (S_CONTROL)(byte3 & 0x0F); };
    S_CONTROL get_pin8() { return (S_CONTROL)((byte3 & 0xF0) >> 4); };
    S_CONTROL get_pin9() { return (S_CONTROL)(byte4 & 0x0F); };
    S_CONTROL get_pin10() { return (S_CONTROL)((byte4 & 0xF0) >> 4); };
    S_CONTROL get_pin11() { return (S_CONTROL)(byte5 & 0x0F); };
    S_CONTROL get_pin12() { return (S_CONTROL)((byte5 & 0xF0) >> 4); };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };

private:
    uint8_t byte0, byte1, byte2, byte3, byte4, byte5;
};