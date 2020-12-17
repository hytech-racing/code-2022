#include <fstream>

#include "BoardDef.h"
#include "CAN_simulator.h"
#include "HTException.h"
#include "Serial.h"
#include "Simulator.h"

#ifdef HYTECH_ARDUINO_TEENSY_32
    #include "Interrupts.h"
#endif

extern void setup(), loop();

unsigned long long Simulator::sys_time = 0;
unsigned long long Simulator::LOOP_PERIOD = 0;
MockPin* Simulator::io = nullptr;

void Simulator::begin(unsigned long long period) {
    if (io)
        throw CustomException("Failed to properly tear down previous simulator");

    LOOP_PERIOD = period;

    Simulator::sys_time = 0;
    io = new MockPin [NUM_PINS + 1];
    for (int i = 0; i <= NUM_PINS; ++i)
        io[i] = MockPin(i);

    setup();
}

void Simulator::teardown() {
    delete [] io;

    Simulator::sys_time = 0;
    Simulator::io = nullptr;
    Serial.end();
    Serial2.end();

    CAN_simulator::purge();
}

void Simulator::next() {
    sys_time += LOOP_PERIOD;
    #ifdef HYTECH_ARDUINO_TEENSY_32
        interrupts::runAll();
    #endif

    loop();
}