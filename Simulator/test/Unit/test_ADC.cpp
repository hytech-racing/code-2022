#include <time.h>

#include "gtest/gtest.h"
#include "MockADC.h"
#include "ADC_SPI.h"
#include "Simulator.h"

void setup(){}
void loop(){}

TEST(UNIT, MockADC) {
	srand(time(0));
	MockADC mock_adc(10);
	ADC_SPI real_adc(10);

	mock_adc.begin();
	Simulator::begin();

	for (int channel = 0; channel < 8; ++channel) {
		uint16_t value = rand() & 0xFFF;
		mock_adc.set(channel, value);
		ASSERT_EQ(value, real_adc.read_adc(channel));
	}

	try {
		mock_adc.set(8, 0);
		FAIL() << "Should have failed to set illegal channel value";
	} catch (HTException) {}

	try {
		mock_adc.set(0, 0x1FFF);
		FAIL() << "Should have failed to set more than 12-bit value";
	} catch (HTException) {}

	mock_adc.teardown();
	Simulator::teardown();
}
