#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

class Reg_Group_PWM {
public:
    Reg_Group_PWM() = default;
    Reg_Group_PWM(uint8_t pwm_all) {
        byte0 = pwm_all << 4 | pwm_all;
        byte1 = pwm_all << 4 | pwm_all;
        byte2 = pwm_all << 4 | pwm_all;
        byte3 = pwm_all << 4 | pwm_all;
        byte4 = pwm_all << 4 | pwm_all;
        byte5 = pwm_all << 4 | pwm_all;
    }
    Reg_Group_PWM(uint8_t pwm1_,
                    uint8_t pwm2_,
                    uint8_t pwm3_,
                    uint8_t pwm4_,
                    uint8_t pwm5_,
                    uint8_t pwm6_,
                    uint8_t pwm7_,
                    uint8_t pwm8_,
                    uint8_t pwm9_,
                    uint8_t pwm10_,
                    uint8_t pwm11_,
                    uint8_t pwm12_) {
        byte0 = (pwm2_ << 4) + pwm1_;
        byte1 = (pwm4_ << 4) + pwm3_;
        byte2 = (pwm6_ << 4) + pwm5_;
        byte3 = (pwm8_ << 4) + pwm7_;
        byte4 = (pwm10_ << 4) + pwm9_;
        byte5 = (pwm12_ << 4) + pwm11_;
    };
    Reg_Group_PWM(uint8_t* byte_arr) : 
        byte0(byte_arr[0]), 
        byte1(byte_arr[1]), 
        byte2(byte_arr[2]), 
        byte3(byte_arr[3]), 
        byte4(byte_arr[4]), 
        byte5(byte_arr[5]) { };


    uint8_t get_pwm1() { return byte0 & 0x0F; };
    uint8_t get_pwm2() { return (byte0 & 0xF0) >> 4; };
    uint8_t get_pwm3() { return byte1 & 0x0F; };
    uint8_t get_pwm4() { return (byte1 & 0xF0) >> 4; };
    uint8_t get_pwm5() { return byte2 & 0x0F; };
    uint8_t get_pwm6() { return (byte2 & 0xF0) >> 4; };
    uint8_t get_pwm7() { return byte3 & 0x0F; };
    uint8_t get_pwm8() { return (byte3 & 0xF0) >> 4; };
    uint8_t get_pwm9() { return byte4 & 0x0F; };
    uint8_t get_pwm10() { return (byte4 & 0xF0) >> 4; };
    uint8_t get_pwm11() { return byte5 & 0x0F; };
    uint8_t get_pwm12() { return (byte5 & 0xF0) >> 4; };

    uint8_t* buf(){
        return reinterpret_cast<uint8_t*>(this);
    };

private:
    uint8_t byte0, byte1, byte2, byte3, byte4, byte5;
};