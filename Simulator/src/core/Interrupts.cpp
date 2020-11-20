#ifdef HYTECH_ARDUINO_TEENSY_32

#include <map>

#include "HTException.h"
#include "Interrupts.h"

namespace interrupts { std::map<int, void(*)()> interruptMap; }
bool interruptsEnabled = false;

void interrupts() { interruptsEnabled = true; }

void NVIC_ENABLE_IRQ(int irq) {
	if (!interruptsEnabled)
		throw CustomException("Teensy 3.2 must manually enable interrupts");
	if (interrupts::interruptMap.find(irq) == interrupts::interruptMap.end())
		interrupts::interruptMap.insert({ irq, []{} });
}

void attachInterruptVector(int irq, void(*interruptVector)()) {
	auto iter = interrupts::interruptMap.find(irq);
	if(iter == interrupts::interruptMap.end())
		throw InterruptNotEnabledException(irq);
	iter->second = interruptVector;
}

#endif