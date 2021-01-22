#include "MCP23S17.h"

#include "SPI.h"

MCP23S17::MCP23S17(uint8_t address, uint8_t cs, unsigned int SPIspeed) {
	_cs = cs;
}

void MCP23S17::begin() {
	SPI.begin();
	pinMode(_cs, OUTPUT);
	digitalWrite(_cs, HIGH);

	_modeCache = 0;
    _pullupCache = 0;
    _outputCache = 0;
}

void MCP23S17::pinMode(uint8_t pin, uint8_t mode) {
	bitWrite(_modeCache, pin, mode);
}

void MCP23S17::pullupMode(uint8_t pin, uint8_t mode) {
	bitWrite(_pullupCache, pin, mode);
}

void MCP23S17::digitalWrite(uint8_t pin, uint8_t value) {
	bitWrite(_outputCache, pin, value == -1 ? bitRead(_pullupCache, pin) : value);
}

void MCP23S17::digitalWrite(unsigned int value) {
	_outputCache &= ~_modeCache;
	_outputCache |= value & _modeCache;
}

unsigned MCP23S17::digitalRead() {
	return _outputCache;
}
