#include "Arduino.h"
#include "SPI.h"

uint8_t SPIClass::initialized = 0;
uint8_t SPIClass::interruptMode = 0;
uint8_t SPIClass::interruptMask = 0;
uint8_t SPIClass::interruptSave = 0;
uint8_t SPIClass::inTransactionFlag = 0;
uint8_t SPIClass::bitOrder = MSBFIRST;

SPIClass SPI;

void SPIClass::begin() {
	initialized = true;
	pinMode(SS, OUTPUT);
	pinMode(MOSI, OUTPUT);
	pinMode(MISO, INPUT);
	pinMode(SCK, RESERVED);

	digitalWrite(SS, HIGH);
	SPDR.begin();
}

void SPIClass::beginTransaction(SPISettings settings) {
	validate();
	if (inTransactionFlag)
		throw SPIException("Began SPI transaction without terminating previous one");
	inTransactionFlag = 1;
	bitOrder = settings.bitOrder;
}

uint16_t SPIClass::transfer16(uint16_t data) {
	validate();
	union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
	in.val = data;
	if (bitOrder == MSBFIRST) {
		SPDR = in.msb;
		out.msb = SPDR;
		SPDR = in.lsb;
		out.lsb = SPDR;
	}
	else {
		SPDR = in.lsb;
		out.lsb = SPDR;
		SPDR = in.msb;
		out.msb = SPDR;
	}
	return out.val;
}

void SPIClass::transfer(void *buf, size_t count) {
	validate();
	if (count == 0) return;
	uint8_t *p = (uint8_t *)buf;
	SPDR = *p;
	while (--count > 0) {
		uint8_t out = *(p + 1);
		uint8_t in = SPDR;
		SPDR = out;
		*p++ = in;
	}
	*p = SPDR;
}

void SPIClass::endTransaction(void) {
	validate();
	if (!inTransactionFlag)
		throw SPIException("SPI attempted to end transaction without beginning one");
	inTransactionFlag = 0;
	bitOrder = MSBFIRST;
}

void SPIClass::end() {
	initialized = false;
	pinMode(SS, UNUSED);
	pinMode(MOSI, UNUSED);
	pinMode(MISO, UNUSED);
	pinMode(SCK, UNUSED);
}