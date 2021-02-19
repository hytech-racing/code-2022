#ifdef HYTECH_ARDUINO_TEENSY_32

#include <map>

#include "HTException.h"
#include "Interrupts.h"

static bool interruptsEnabled = false;

namespace Interrupts { 
	static std::map<int, void(*)()> interruptMap;
	void runAll() { for (auto &it : interruptMap) it.second(); }
	bool enabled() { return interruptsEnabled; }
}

void interrupts() { interruptsEnabled = true; }
void noInterrupts() { interruptsEnabled = false; }

void NVIC_ENABLE_IRQ(int irq) {
	if (!interruptsEnabled)
		throw SetupException("Teensy 3.2 must manually enable interrupts");
	if (Interrupts::interruptMap.find(irq) == Interrupts::interruptMap.end())
		Interrupts::interruptMap.insert({ irq, []{} });
}

void attachInterruptVector(int irq, void(*interruptVector)()) {
	auto iter = Interrupts::interruptMap.find(irq);
	if(iter == Interrupts::interruptMap.end())
		throw SetupException("Interrupt vector %d not enabled", irq);
	iter->second = interruptVector;
}

#endif