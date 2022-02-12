#ifndef __ADC_SPI_H__
#define __ADC_SPI_H__

#include <stdint.h>
#include <stddef.h>

#define DEFAULT_SPI_CS 10
#define DEFAULT_SPI_SPEED 1000000

class ADC_SPI {
	public:
		ADC_SPI();
		ADC_SPI(int CS);
        ADC_SPI(int CS, unsigned int SPIspeed);
		void init(int CS, unsigned int SPIspeed);
		uint16_t read_adc(int channel);
	private:
		int ADC_SPI_CS;
        unsigned int SPI_SPEED;
};


#endif // !<ADC_SPI.H>
#pragma once
