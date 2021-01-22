#pragma once
#include "ADC_SPI.h"

namespace MockADC {
	void write(int CS, int channel, uint16_t value);
	void purge();
}