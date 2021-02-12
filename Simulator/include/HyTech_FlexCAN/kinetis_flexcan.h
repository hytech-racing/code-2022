#pragma once
#include "BoardDef.h"

#ifdef HYTECH_ARDUINO_TEENSY_32
	#define IRQ_CAN_MESSAGE 29
	extern unsigned long long FLEXCAN0_IMASK1;
	#define FLEXCAN_IMASK1_BUF5M (0x00000020)
#elif defined(HYTECH_ARDUINO_TEENSY_35)
  	extern unsigned long long FLEXCAN0_MCR;
#endif