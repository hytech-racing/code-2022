#pragma once

#include <string>

#include "MockPin.h"


class Simulator {
public:
    Simulator(unsigned long long period = 0);
    ~Simulator();
    void next();

    inline bool digitalRead(int pin)                { return io[pin].fOutputValue; };
    inline unsigned analogRead(int pin)             { return io[pin].fOutputValue; }
    inline void digitalWrite(int pin, bool value)   { io[pin].fInputValue = value; }
    inline void analogWrite(int pin, unsigned value){ io[pin].fInputValue = value; }
    inline int getPinMode(int pin)                  { return io[pin].fMode; }
    
    // used to manage pin i/o
    friend bool digitalRead(int pin);
    friend unsigned analogRead(int pin);
    friend void digitalWrite(int pin, bool value);
    friend void analogWrite(int pin, unsigned value);
    friend void pinMode(int pin, unsigned mode);

    // used for time management
    friend unsigned long long millis();
    friend void delay (unsigned long long time);
private:
    static unsigned long long sys_time;
    unsigned long long LOOP_PERIOD;
    static MockPin* io;
};
