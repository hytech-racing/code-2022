#pragma once

#include <string>

#include "MockPin.h"

class Simulator {
public:
    static void begin(unsigned long long period = 0);
    static void teardown();
    static void next();

    static inline bool digitalRead(int pin)                { return io[pin].fOutputValue; };
    static inline unsigned analogRead(int pin)             { return io[pin].fOutputValue; }
    static inline void digitalWrite(int pin, bool value)   { io[pin].fInputValue = value; }
    static inline void analogWrite(int pin, unsigned value){ io[pin].fInputValue = value; }
    static inline int getPinMode(int pin)                  { return io[pin].fMode; }
    
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
    Simulator();
    static unsigned long long sys_time;
    static unsigned long long LOOP_PERIOD;
    static MockPin* io;
};
