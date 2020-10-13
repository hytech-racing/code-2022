#pragma once
#include <Metro.h>

typedef struct _DebouncedButton {
    bool pressed;
    bool debouncing;
    Metro metro;

    _DebouncedButton(unsigned long interval);
    bool update(bool);
} DebouncedButton;

DebouncedButton::_DebouncedButton(unsigned long interval) : metro(interval) {
    pressed = false;
    debouncing = false;
}

bool DebouncedButton::update(bool reading) {
    if (reading == pressed && !debouncing) {
        debouncing = true;
        metro.reset();
    }
    if (debouncing && reading != pressed)
        debouncing = false;
    if (debouncing && metro.check()) {
        pressed = !pressed;
        return pressed;
    }
    return false;
}