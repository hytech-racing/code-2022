#pragma once

#ifdef HYTECH_ARDUINO_TEENSY_32

#include <map>

#include "HTException.h"

void interrupts();
void noInterrupts();
void NVIC_ENABLE_IRQ(int irq);
void attachInterruptVector(int irq, void(*interruptVector)());

namespace Interrupts {
	bool enabled();
	void runAll();
}

#endif