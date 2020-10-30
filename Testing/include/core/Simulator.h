#pragma once

#include <string>
#include "MockPin.h"

extern MockPin* io;

class Simulator {
public:
    Simulator(unsigned long long period);
    ~Simulator();
    void next();
    friend unsigned long long millis();
    friend void delay(unsigned long long);
private:
    static unsigned long long sys_time;
    const unsigned long long LOOP_PERIOD;
};