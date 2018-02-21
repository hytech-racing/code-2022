#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>
#include <stddef.h>

class Teensy_SPI {
	public:
		Teensy_SPI(int SCK, int CS, int DOUT, int DIN);
		uint16_t read_adc(int channel);
	private:
		int ADC_SPI_SCK;
		int ADC_SPI_CS;
		int ADC_SPI_DOUT;
		int ADC_SPI_DIN;
};


#endif // !<SPI.H>
#pragma once
