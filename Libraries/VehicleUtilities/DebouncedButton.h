#pragma once
#include "Metro.h"
#include "Arduino.h" // ETHAN: DELET THIS

class DebouncedButton {
public:
    DebouncedButton() : metro(0) {}

    void begin(int p, unsigned long interval, int mode = INPUT_PULLUP) {
        pin = p;
        pinMode(pin, mode);
        pressed = false;
        reading = (mode == INPUT_PULLUP);
        debouncing = false;
        metro = Metro(interval);
    }

    bool check() {
        reading = digitalRead(pin);
        if (reading == pressed && !debouncing) {
            debouncing = true;
            metro.reset();
        }
        if (debouncing && reading != pressed)
            debouncing = false;
        if (debouncing && metro.check()) {
            pressed = !pressed;
        }
        return pressed;
    }

private:
    bool pressed;
    bool reading;
    bool debouncing;
    int pin;
    Metro metro;
};


