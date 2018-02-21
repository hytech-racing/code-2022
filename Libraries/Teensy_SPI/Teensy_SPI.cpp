#include "Teensy_SPI.h"
#include <SPI.h>

Teensy_SPI::Teensy_SPI(int SCK, int CS, int DOUT, int DIN) {
	ADC_SPI_SCK = SCK;
	ADC_SPI_CS = CS;
	ADC_SPI_DOUT = DOUT;
	ADC_SPI_DIN = DIN;

	pinMode(ADC_SPI_CS, OUTPUT);
	pinMode(ADC_SPI_CS, HIGH);

	pinMode(ADC_SPI_SCK, OUTPUT);
	pinMode(ADC_SPI_DOUT, OUTPUT);
	pinMode(ADC_SPI_DIN, INPUT);

	// initialize SPI:
	SPI.begin();
}

uint16_t Teensy_SPI::read_adc(int channel) {
	// gain control of the SPI port
	// and configure settings
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

	// take the SS pin low to select the chip:
	digitalWrite(ADC_SPI_CS, LOW);

	// set up channel
	byte b = B01100000;
	b |= ((channel << 2));

	//send in the channel via SPI:
	SPI.transfer(b);

	// read data from SPI
	byte result1 = SPI.transfer(0);
	byte result2 = SPI.transfer(0);

	// take the SS pin high to de-select the chip:
	digitalWrite(ADC_SPI_CS, HIGH);

	// release control of the SPI port
	SPI.endTransaction();

	return (result1 << 4) | (result2 >> 4);
}