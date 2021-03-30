#pragma once
#include <stdint.h>

class Listener {
public:
	virtual void begin() = 0;
	virtual void teardown() = 0;
	virtual void process(uint8_t pinNo, unsigned value) = 0;
};