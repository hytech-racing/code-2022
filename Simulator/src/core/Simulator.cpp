#include <BoardDef.h>
#include <fstream>
#include <HTException.h>
#include <Simulator.h>
#include <Serial.h>

#ifdef HYTECH_ARDUINO_TEENSY_32
    #include <Interrupts.h>
    #include <map>

    extern std::map<int, void(*)()> interruptMap;
#endif

extern void setup(), loop();

MockPin* io;
extern MockSerial Serial, Serial2;
unsigned long long Simulator::sys_time = 0;

Simulator::Simulator(unsigned long long period) : LOOP_PERIOD(period) {
    Simulator::sys_time = 0;
    io = new MockPin [NUM_PINS + 1];
    for (int i = 0; i <= NUM_PINS; ++i)
        io[i] = MockPin(i);

    setup();
}

Simulator::~Simulator() {
    Simulator::sys_time = 0;
    delete [] io;
    Serial.end();
    Serial2.end();
}

void Simulator::next() {
    sys_time += LOOP_PERIOD;
    #if HT_ARDUINO_TEENSY_32
        for (auto &it : interruptMap) it.second();
    #endif

    loop();
}