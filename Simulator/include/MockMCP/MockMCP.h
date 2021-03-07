#pragma once

#include <MCP23S17.h>
#include "SPIListener.h"

#define OPCODE_WRITE (0b01000000)
#define OPCODE_READ  (0b01000001)

#define UNKWNA 0
#define UNKWNB 1

class MockMCP : public SPIListener {
	typedef enum { IDLE, SELECT_REGISTER, READ_OR_TRANSMIT } State;

public:
	MockMCP(int address, int cs) : SPIListener(cs), address(address) {}

	inline void begin() {
		SPIListener::begin();
		memset(values, 0, 22);
		memset(unknown, 0, 2);
		state = IDLE;
	}

	void processByte(byte);

	inline void setGPIOValue(uint8_t channel, bool value) {
		if (channel >= 16) throw MCPException("GPIO", channel);
		setGPIOUnknown(channel, false);
		bitWrite(values[GPIOA + (channel >= 8)], channel & 0x7, value);
	}

	inline void setGPIOUnknown(uint8_t channel, bool value) {
		if (channel >= 16) throw MCPException("GPIO", channel);
		bitWrite(unknown[channel >= 8], channel & 0x7, value);
	}

	inline bool getOLATValue(uint8_t channel) {
		if (channel >= 16) throw MCPException("OLAT", channel);
		return bitRead(values[OLATA + (channel >= 8)], channel & 0x7);
	}
	
private:
	uint8_t address, opcode, targetRegister;
	uint8_t values [22];
	uint8_t unknown [2];
	State state;
};
