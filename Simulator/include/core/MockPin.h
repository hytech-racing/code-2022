#pragma once

#include <fstream>
#include <string>

#define UNUSED -1
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define RESERVED 3

class MockPin {
public:
    MockPin();
    MockPin(int pin);

    unsigned vehicle_read();
    void vehicle_write(unsigned value);
    void vehicle_pinMode(int mode);

    friend class Simulator;
private:
    int fPin;
    int fInputValue = -1;
    int fOutputValue = -1;
    int fMode = UNUSED;
};

std::string decodePinMode(int mode);