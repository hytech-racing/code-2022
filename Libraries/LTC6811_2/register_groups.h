#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

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
                    S_CONTROL pin12_) :
        pin1(pin1_),
        pin2(pin2_),
        pin3(pin3_),
        pin4(pin4_),
        pin5(pin5_),
        pin6(pin6_),
        pin7(pin7_),
        pin8(pin8_),
        pin9(pin9_),
        pin10(pin10_),
        pin11(pin11_),
        pin12(pin12_) { };

    S_CONTROL get_pin1() { return pin1; };
    S_CONTROL get_pin2() { return pin2; };
    S_CONTROL get_pin3() { return pin3; };
    S_CONTROL get_pin4() { return pin4; };
    S_CONTROL get_pin5() { return pin5; };
    S_CONTROL get_pin6() { return pin6; };
    S_CONTROL get_pin7() { return pin7; };
    S_CONTROL get_pin8() { return pin8; };
    S_CONTROL get_pin9() { return pin9; };
    S_CONTROL get_pin10() { return pin1; };
    S_CONTROL get_pin11() { return pin11; };
    S_CONTROL get_pin12() { return pin12; };

private:
    S_CONTROL pin1;
    S_CONTROL pin2;
    S_CONTROL pin3;
    S_CONTROL pin4;
    S_CONTROL pin5;
    S_CONTROL pin6;
    S_CONTROL pin7;
    S_CONTROL pin8;
    S_CONTROL pin9;
    S_CONTROL pin10;
    S_CONTROL pin11;
    S_CONTROL pin12;
};

class Reg_Group_PWM {
public:
    Reg_Group_PWM() = default;
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
                    uint8_t pwm12_) :
        pwm1(pwm1_),
        pwm2(pwm2_),
        pwm3(pwm3_),
        pwm4(pwm4_),
        pwm5(pwm5_),
        pwm6(pwm6_),
        pwm7(pwm7_),
        pwm8(pwm8_),
        pwm9(pwm9_),
        pwm10(pwm10_),
        pwm11(pwm11_),
        pwm12(pwm12_) { };

    uint8_t get_pwm1() { return pwm1; };
    uint8_t get_pwm2() { return pwm2; };
    uint8_t get_pwm3() { return pwm3; };
    uint8_t get_pwm4() { return pwm4; };
    uint8_t get_pwm5() { return pwm5; };
    uint8_t get_pwm6() { return pwm6; };
    uint8_t get_pwm7() { return pwm7; };
    uint8_t get_pwm8() { return pwm8; };
    uint8_t get_pwm9() { return pwm9; };
    uint8_t get_pwm10() { return pwm10; };
    uint8_t get_pwm11() { return pwm11; };
    uint8_t get_pwm12() { return pwm12; };

private:
    uint8_t pwm1;
    uint8_t pwm2;
    uint8_t pwm3;
    uint8_t pwm4;
    uint8_t pwm5;
    uint8_t pwm6;
    uint8_t pwm7;
    uint8_t pwm8;
    uint8_t pwm9;
    uint8_t pwm10;
    uint8_t pwm11;
    uint8_t pwm12;
};