#pragma once

#ifdef HYTECH_ARDUINO_TEENSY_32

#include <map>

#include "HTException.h"

void interrupts();
void NVIC_ENABLE_IRQ(int irq);
void attachInterruptVector(int irq, void(*interruptVector)());

namespace interrupts {
	extern std::map<int, void(*)()> interruptMap;
	inline void runAll() { for (auto &it : interruptMap) it.second(); }
}

#endif