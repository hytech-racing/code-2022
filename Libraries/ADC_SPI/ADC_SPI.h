#ifndef __ADC_SPI_H__
#define __ADC_SPI_H__

#include <stdint.h>
#include <stddef.h>

#define DEFAULT_SPI_CS 10

class ADC_SPI {
	public:
		ADC_SPI();
		ADC_SPI(int CS);
		void init(int CS);
		uint16_t read_adc(int channel);
	private:
		int ADC_SPI_CS;
};


#endif // !<ADC_SPI.H>
#pragma once
