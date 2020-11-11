#pragma once

#include <string>
#include "MockPin.h"

extern MockPin* io;

class Simulator {
public:
    Simulator(unsigned long long period = 0);
    ~Simulator();
    void next();
    friend unsigned long long millis();
    friend void delay(unsigned long long);

    inline bool digitalRead(int pin) { return io[pin].fOutputValue; };
    inline void digitalWrite(int pin, bool value) { io[pin].fInputValue = value; }
    inline unsigned analogRead(int pin) { return io[pin].fOutputValue; }
    inline void analogWrite(int pin, unsigned value) { io[pin].fInputValue = value; }
    inline int getPinMode(int pin) { return io[pin].fMode; }

private:
    static unsigned long long sys_time;
    const unsigned long long LOOP_PERIOD;
};