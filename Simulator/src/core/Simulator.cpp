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
MockPin* Simulator::io = nullptr;

Simulator::Simulator(unsigned long long period) : LOOP_PERIOD(period) {
    if (io)
        throw CustomException("No more than one simulator can be open at any given time");

    Simulator::sys_time = 0;
    io = new MockPin [NUM_PINS + 1];
    for (int i = 0; i <= NUM_PINS; ++i)
        io[i] = MockPin(i);

    setup();
}

Simulator::~Simulator() {
    delete [] io;

    Simulator::sys_time = 0;
    Simulator::io = nullptr;
    Serial.end();
    Serial2.end();
}

void Simulator::next() {
    sys_time += LOOP_PERIOD;
    #ifdef HYTECH_ARDUINO_TEENSY_32
        interrupts::runAll();
    #endif

    loop();
}