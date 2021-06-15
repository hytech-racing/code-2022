#pragma once

#ifdef HYTECH_ARDUINO_TEENSY_32
	#include "BoardDefs/Teensy_32.h"
#elif defined(HYTECH_ARDUINO_TEENSY_35)
	#include "BoardDefs/Teensy_35.h"
#elif defined(HYTECH_ARDUINO_TEENSY_40)
	#include "BoardDefs/Teensy_40.h"
#elif defined(HYTECH_ARDUINO_UNO)
	#include "BoardDefs/Uno.h"
#else
	#error "Failed to specify valid microcontroller"
#endif