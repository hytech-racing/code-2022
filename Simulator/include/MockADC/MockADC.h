#pragma once

#include "Arduino.h"
#include "Simulator.h"
#include "SPI.h"
#include "SPIListener.h"

class MockADC : public SPIListener {
	typedef enum { IDLE, SEND_MSB, SEND_LSB } State;

public:
	MockADC(int cs) : SPIListener(cs), state(IDLE) {}

	inline void begin() {
		SPIListener::begin();
		memset(channels, 0, 8 * sizeof(uint16_t));
		state = IDLE;
	}

	inline void processByte(byte value) {
		if (state == IDLE)				currentChannel = (value & ~B01100000) >> 2;
		else if (state == SEND_MSB)		write(channels[currentChannel] >> 4);
		else							write((channels[currentChannel] & 0xF) << 4);
		
		state = (State) ((state + 1) % (SEND_LSB + 1));
	}

	inline void set(int channel, uint16_t value) {
		if (value > 0xFFF)
			throw HTException("Failed to set ADC Channel Value", "ADC value has maximum 12 bits of precision");
		if (channel > 7)
			throw HTException("Failed to set ADC Channel Value", "ADC channel must be in range 0-7, inclusive");
		channels[channel] = value;
	}
	
private:
	int cs, currentChannel;
	uint16_t channels [8];
	State state;
};