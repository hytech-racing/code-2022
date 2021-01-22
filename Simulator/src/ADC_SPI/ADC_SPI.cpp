#include "ADC_SPI.h"
#include "Arduino.h"

#include <map>

std::map<int, uint16_t[8]> channelStore;

ADC_SPI::ADC_SPI() {
	init(DEFAULT_SPI_CS, DEFAULT_SPI_SPEED);
}

ADC_SPI::ADC_SPI(int CS) {
	init(CS, DEFAULT_SPI_SPEED);
}

ADC_SPI::ADC_SPI(int CS, unsigned int SPIspeed) {
	init(CS, SPIspeed);
}

void ADC_SPI::init(int CS, unsigned int SPIspeed) {
    ADC_SPI_CS = CS;
	SPI_SPEED  = SPIspeed;
	pinMode(CS, RESERVED);
}

uint16_t ADC_SPI::read_adc(int channel) {
	return channelStore[ADC_SPI_CS][channel];
}

namespace Mock_ADC_SPI {
	void write(int CS, int channel, uint16_t value) {
		channelStore[CS][channel] = value;
	}

	void purge() {
		channelStore.clear();
	}
}