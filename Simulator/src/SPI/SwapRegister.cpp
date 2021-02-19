#include "SPI.h"
#include "SPIListener.h"

SPIUtils::SwapRegister SPDR;

namespace SPIUtils {
	SwapRegister& SwapRegister::operator=(uint8_t& other) {
		if (SPIListener::selected) {
			SPIListener::selected->processByte(other);
			value = SPIListener::selected->response;
			return *this;
		}

		value = 0;
		if (SPI.bitOrder == MSBFIRST)
			for (int i = 0x80; i; i >>= 1)
				digitalWrite(MOSI, other & i);
		else
			for (int i = 1; i <= 0x80; i <<= 1)
				digitalWrite(MOSI, other & i);
		other = value;
		return *this;
	}


	void SwapRegister::process(uint8_t pinNo, unsigned) {
		static int spiCount = 0;
		
		if (pinNo != MISO) return;

		if (SPI.bitOrder == MSBFIRST) value = (value << 1) | digitalRead(MISO);
		else value |= (digitalRead(MISO) << spiCount);

		++spiCount;
		spiCount &= 7;
	}
}