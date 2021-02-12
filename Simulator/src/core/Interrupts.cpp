#ifdef HYTECH_ARDUINO_TEENSY_32

#include <map>

#include "HTException.h"
#include "Interrupts.h"

namespace Interrupt { static std::map<int, void(*)()> interruptMap; }
static bool interruptsEnabled = false;

void interrupts() { interruptsEnabled = true; }
void noInterrupts() { interruptsEnabled = false; }

void NVIC_ENABLE_IRQ(int irq) {
	if (!interruptsEnabled)
		throw SetupException("Teensy 3.2 must manually enable interrupts");
	if (Interrupt::interruptMap.find(irq) == Interrupt::interruptMap.end())
		Interrupt::interruptMap.insert({ irq, []{} });
}

void attachInterruptVector(int irq, void(*interruptVector)()) {
	auto iter = Interrupt::interruptMap.find(irq);
	if(iter == Interrupt::interruptMap.end())
		throw SetupException("Interrupt vector %d not enabled", irq);
	iter->second = interruptVector;
}

#endif