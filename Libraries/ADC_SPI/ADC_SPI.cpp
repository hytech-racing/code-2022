#include "ADC_SPI.h"
#include <SPI.h>

/*
 * This library is used to communicate with
 * the Microchip MCP3208 12-bit ADC using
 * the Teensy/Arduino SPI library
 */

/*
 * Initialize ADC SPI using default CS pin
 */
ADC_SPI::ADC_SPI() {
	ADC_SPI(DEFAULT_SPI_CS);
}

/*
 * Initialize ADC SPI using custom CS pin
 * param CS Pin to use for Chip Select
 */
ADC_SPI::ADC_SPI(int CS) {
	ADC_SPI_CS = CS;

	pinMode(ADC_SPI_CS, OUTPUT);
	pinMode(ADC_SPI_CS, HIGH);

	// Initialize SPI:
	SPI.begin();
}

/*
 * Measure an ADC channel
 * param channel MCP3208 channel to read
 * return 0-5V measurement scaled to 0-4095
 */
uint16_t ADC_SPI::read_adc(int channel) {
	// Gain control of the SPI port
	// and configure settings
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

	// Take the SS pin low to select the chip:
	digitalWrite(ADC_SPI_CS, LOW);

	// Set up channel
	byte b = B01100000;
	b |= ((channel << 2));

	// Send in the channel via SPI:
	SPI.transfer(b);

	// Read data from SPI
	byte result1 = SPI.transfer(0);
	byte result2 = SPI.transfer(0);

	// Take the SS pin high to de-select the chip:
	digitalWrite(ADC_SPI_CS, HIGH);

	// Release control of the SPI port
	SPI.endTransaction();

	return (result1 << 4) | (result2 >> 4);
}