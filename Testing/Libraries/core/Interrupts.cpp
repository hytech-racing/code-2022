#include <BoardDef.h>

#ifdef HYTECH_ARDUINO_TEENSY_32

#include "Interrupts.h"
#include <HTException.h>
#include <map>

std::map<int, void(*)()> interruptMap;
bool interruptsEnabled = false;

void interrupts() { interruptsEnabled = true; }

void NVIC_ENABLE_IRQ(int irq) {
	if (!interruptsEnabled)
		throw CustomException("Teensy 3.2 must manually enable interrupts");
	if (interruptMap.find(irq) == interruptMap.end())
		interruptMap.insert({ irq, []{} });
}

void attachInterruptVector(int irq, void(*interruptVector)()) {
	auto iter = interruptMap.find(irq);
	if(iter == interruptMap.end())
		throw InterruptNotEnabledException(irq);
	iter->second = interruptVector;
}

#endif