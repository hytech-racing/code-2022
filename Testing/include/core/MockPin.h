#pragma once

#include "fstream"
#include "string"

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define RESERVED 3

class MockPin {
public:
    MockPin();
    MockPin(int pin);
    unsigned sim_value();
    void sim_write(unsigned value);
    void sim_pinMode(int mode);
    int value();
    void setValue(unsigned value);
    void setInputToPullup();
    int pinMode();
private:
    int fPin;
    int fInputValue = -1;
    int fOutputValue = -1;
    int fMode = -1;
};

std::string decodePinMode(int mode);