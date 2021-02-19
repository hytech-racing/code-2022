#pragma once

#include <fstream>
#include <list>
#include <string>

#include "Listener.h"

#define UNUSED -1
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define RESERVED 3

class MockPin {
public:
    MockPin();
    MockPin(int pin);

    inline void teardown() {
        fInputValue = fOutputValue = fMode = UNUSED;
        listeners.clear();
    }
    unsigned vehicle_read();
    void vehicle_write(unsigned value);
    void vehicle_pinMode(int mode);

    friend class Simulator;
private:
    int fPin;
    int fInputValue = UNUSED;
    int fOutputValue = UNUSED;
    int fMode = UNUSED;
    std::list<Listener*> listeners;

    inline void sim_setValue(unsigned val) {
        fInputValue = val;
        for (Listener* listener : listeners)
            listener->process(fPin, fInputValue);
    }
};
