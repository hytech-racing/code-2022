#pragma once
#include <Metro.h>

enum class BLINK_MODES { OFF = 0, ON = 1, FAST = 2, SLOW = 3};
const int BLINK_RATES[4] = { 0, 0, 150, 400}; // OFF, ON, FAST, SLOW, FASTER

class VariableLED {
private:
    Metro blinker;
    int pin;
    BLINK_MODES mode;
    bool led_value = false;
    bool pwm = false;
    uint8_t pwm_speed = 0;

public:
    VariableLED(int p, bool metro_should_autoreset = true, bool pwm = false, uint8_t pwm_speed = 0) :
        blinker(0, metro_should_autoreset),
        pin(p),
        pwm(pwm),
        pwm_speed(pwm_speed) {};

    void setMode(BLINK_MODES m) {
        if (mode == m)
            return;
        mode = m;
        if (BLINK_RATES[(int)m]) {
            blinker.interval(BLINK_RATES[(int)m]);
            blinker.reset();
        }
    }

    void update() {
        if (mode == BLINK_MODES::OFF){
            if (pwm)
                analogWrite(pin, led_value = LOW);
            else
                digitalWrite(pin, led_value = LOW);
        }
        else if (mode == BLINK_MODES::ON) {
            if (pwm){
                analogWrite(pin, pwm_speed);
                led_value = HIGH;
            }
            else
                digitalWrite(pin, led_value = HIGH);
        }
        else if (blinker.check()) { // blinker mode
            if (pwm){
                if (led_value = !led_value)
                    analogWrite(pin, pwm_speed);
                else
                    analogWrite(pin, 0);
            }
            else
                digitalWrite(pin, led_value = !led_value);
        }
    }

    BLINK_MODES getMode() { return mode; }
};
