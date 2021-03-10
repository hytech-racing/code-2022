#include <fstream>

#include "BoardDef.h"
#include "HTException.h"
#include "Serial.h"
#include "Simulator.h"

#ifdef HYTECH_ARDUINO_TEENSY_32
    #include "Interrupts.h"
#endif

extern void setup(), loop();

unsigned long long Simulator::sys_time = 0;
unsigned long long Simulator::sys_us = 0;
unsigned long long Simulator::LOOP_PERIOD = 0;
MockPin Simulator::io [NUM_PINS + 1];
bool Simulator::running = false;

void Simulator::begin(unsigned long long period) {
    if (running)
        throw SetupException("Failed to properly tear down previous simulator");

    LOOP_PERIOD = period;

    running = true;
    Simulator::sys_time = 0;
    Simulator::sys_us = 0;
    for (int i = 1; i <= NUM_PINS; ++i)
        io[i].fPin = i;

    setup();
}

void Simulator::teardown() {
    if (!running)
        throw HTException("Invalid Simulator Configuration", "Attempted to tear down inactive simulator");
    running = false;
    for (int i = 1; i <= NUM_PINS; ++i)
        io[i] = MockPin(i);

    Simulator::sys_time = 0;
    Simulator::sys_us = 0;
    Serial.end();
    running = false;
}

void Simulator::next() {
    sys_time += LOOP_PERIOD;
    #ifdef HYTECH_ARDUINO_TEENSY_32
        Interrupts::runAll();
    #endif

    loop();
}