#pragma once

#include "Arduino.h"

#define SPI_HAS_TRANSACTION 1
#define SPI_HAS_NOTUSINGINTERRUPT 1
#define SPI_ATOMIC_VERSION 1

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

#define LSBFIRST 0
#define MSBFIRST 1

#define B01100000 0b01100000 // ADC Constant

// We don't have to worry about volatile values so we fix the status registers as ready
#define SPSR 1
#define SPIF 1
#define _BV(SPIF) SPIF

class SPIClass;

namespace SPIUtils {

typedef struct VolatileRegister : public Listener {
	inline void begin() { Simulator::attachListener(MISO, this); }
	inline void teardown() {}
	inline operator uint8_t() { return value; }
	inline VolatileRegister& operator=(int8_t& other) { *this = (uint8_t&) other; }

	VolatileRegister& operator=(uint8_t& other);
	void process(uint8_t pinNo, unsigned value);
private:
	uint8_t value = 0;
} VolatileRegister;

}

extern SPIUtils::VolatileRegister SPDR;

class SPISettings {
public:
	SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) { this->bitOrder = bitOrder; }
	friend class SPIClass;
private:
	uint8_t bitOrder;
};

class SPIClass {
public:
	static void begin();
	static void beginTransaction(SPISettings settings);

	// Write to the SPI bus (MOSI pin) and also receive (MISO pin)
  	inline static uint8_t transfer(uint8_t data) { validate(); return SPDR = data; }

	static uint16_t transfer16(uint16_t data);
	static void transfer(void *buf, size_t count);

	static void endTransaction(void);
	static void end();

	static void setBitOrder(uint8_t _bitOrder) { validate(); bitOrder = _bitOrder; }
	static void setDataMode(uint8_t dataMode) { validate(); }	
	static void setClockDivider(uint8_t clockDiv) { validate(); }

private:
	static uint8_t initialized;
	static uint8_t interruptMode; // 0=none, 1=mask, 2=global
	static uint8_t interruptMask; // which interrupts to mask
	static uint8_t interruptSave; // temp storage, to restore state
	static uint8_t inTransactionFlag;
	static uint8_t bitOrder;

	friend class SPIUtils::VolatileRegister;

	inline static void validate() { if (!initialized) throw SPIException("SPI not initialized"); }
};

extern SPIClass SPI;