#pragma once
#include <Metro.h>

enum class BLINK_MODES { OFF = 0, ON = 1, FAST = 2, SLOW = 3 };
const int BLINK_RATES[4] = { 0, 0, 150, 400 }; // OFF, ON, FAST, SLOW

typedef struct VariableLED {

    VariableLED(int p, bool metro_should_autoreset = true, uint8_t brightness = 255) : 
        blinker(0, metro_should_autoreset),
        pin(p),
        BRIGHTNESS(brightness) {};

    BLINK_MODES getMode() { return mode; }

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
        if (mode == BLINK_MODES::OFF)
            analogWrite(pin, value = LOW);
        else if (mode == BLINK_MODES::ON)
            analogWrite(pin, value = BRIGHTNESS);
        else if (blinker.check()) // blinker mode
            analogWrite(pin, value ^= BRIGHTNESS);
    }

private:
    Metro blinker;
    int pin;
    BLINK_MODES mode;
    const uint8_t BRIGHTNESS;
    uint8_t value = 0;

} VariableLED;