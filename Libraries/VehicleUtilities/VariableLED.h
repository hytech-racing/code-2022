#pragma once
#include <Metro.h>
#include <PackedValues.h>

enum BLINK_MODES { OFF = 0, ON = 1, FAST = 2, SLOW = 3 };
const int BLINK_RATES[4] = { 0, 0, 150, 400 }; // OFF, ON, FAST, SLOW

typedef struct _VariableLED {
    int pin;
    Metro blinker;
    int mode;
    bool led_value = false;

    _VariableLED(int p, bool metro_should_autoreset = true) : blinker(0, metro_should_autoreset) {pin = p; }

    void setMode(int m) {
        if (mode == m)
            return;
        mode = m;
        if (BLINK_RATES[m]) {
            blinker.interval(BLINK_RATES[m]);
            blinker.reset();
        }
    }

    void update(PackedValues& values) {
        if (mode == OFF)
        {
            digitalWrite(pin, LOW);
            led_value = OFF;
        }
        else if (mode == ON)
        {
            digitalWrite(pin, HIGH);
            led_value = ON;
        }
        else if (blinker.check()) // blinker mode
        {
            led_value = !led_value;
            digitalWrite(pin, led_value);
        }
            
    }
} VariableLED;