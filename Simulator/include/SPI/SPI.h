#pragma once

#include "Arduino.h"
#include "BoardDef.h"

#define LSBFIRST 0
#define MSBFIRST 1

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

#define SPI_MODE_MASK 0x0C
#define SPI_CLOCK_MASK 0x03
#define SPI_2XCLOCK_MASK 0x01

#define B01100000 0b01100000;

// hardcode status registers as ready
#define SPSR true
#define SPIF true
#define _BV(SPIF) SPIF

extern int8_t SPDR;

class SPISettings {
public:
	SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {}
	SPISettings() {}
private:
	uint8_t spcr;
	uint8_t spsr;
	friend class SPIClass;
};

class SPI_t {
public:
	inline static void begin() {
			pinMode(SS, OUTPUT);
			digitalWrite(SS, HIGH);

			pinMode(MOSI, RESERVED);
			pinMode(MISO, RESERVED);
			pinMode(SCK, RESERVED);
	}
	static void usingInterrupt(uint8_t interruptNumber);
	static void beginTransaction(SPISettings settings);
	static uint8_t transfer(uint8_t data);
	static uint16_t transfer16(uint16_t data);
	static void transfer(void *buf, size_t count);
	static void setTransferWriteFill(uint8_t ch);
	static void transfer(const void * buf, void * retbuf, uint32_t count);
	static void endTransaction(void);
	inline static void end() { pinMode(SS, UNUSED); }
	static void setBitOrder(uint8_t bitOrder);
	static void setDataMode(uint8_t dataMode);
	inline static void setClockDivider(uint8_t clockDiv) {}
	static void attachInterrupt();
	static void detachInterrupt();

private:
	static uint8_t interruptMode; // 0=none, 1=mask, 2=global
	static uint8_t interruptMask; // which interrupts to mask
	static uint8_t interruptSave; // temp storage, to restore state
	static uint8_t _transferWriteFill;
};

extern SPI_t SPI;