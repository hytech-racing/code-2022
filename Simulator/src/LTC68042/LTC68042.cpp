#include "MockLTC.h"
#include "LTC68042.h"

#include <algorithm>

#include "Linduino.h"
#include "Metro.h"

namespace MockLTC {
	std::queue<uint8_t(*)[6]> cfg_list;
	ADC_SETTINGS adc;

	Metro cellVoltageValid(202), auxVoltageValid(202);
	uint16_t(*cellVoltages)[12], (*auxVoltages)[6];
	int numIC;

	void init (int ic) {
		numIC = ic;
		cellVoltages = new uint16_t[numIC][12];
		auxVoltages = new uint16_t[numIC][6];
	}

	void setCellVoltages(uint16_t(*voltages)[12]) {
		memcpy(cellVoltages, voltages, numIC * 12 * sizeof(uint16_t));
	}

	void setAuxVoltages(uint16_t(*voltages)[6]) {
		memcpy(auxVoltages, voltages, numIC * 6 * sizeof(uint16_t));
	}

	void teardown() {
		while (cfg_list.size()) {
			delete cfg_list.front();
			cfg_list.pop();
		}
		delete [] cellVoltages;
		delete [] auxVoltages;
	}
};

void wakeup_idle() {
	output_low(LTC6804_CS);
	delayMicroseconds(10);
	output_high(LTC6804_CS);
}

void wakeup_sleep() {
	output_low(LTC6804_CS);
	delay(1);
	output_high(LTC6804_CS);
}

void set_adc(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG) {
	MockLTC::adc = { MD, DCP, CH, CHG };
}

void LTC6804_wrcfg(uint8_t total_ic,uint8_t config[][6]) {
	uint8_t (*copy)[6] = new uint8_t[total_ic][6];
	memcpy(copy, config, total_ic * 6 * sizeof(uint8_t));
	MockLTC::cfg_list.push(copy);
}

void LTC6804_adcv() { MockLTC::cellVoltageValid.reset(); }
void LTC6804_adax() { MockLTC::auxVoltageValid.reset(); }

template<typename T>
inline void read(uint8_t reg, uint8_t total_ic, uint8_t MAX_REG, T dest, T src) {
	constexpr uint8_t CELL_IN_REG = 3;
    for (uint8_t cell = std::max(reg - 1, 0) * CELL_IN_REG; cell < std::min(reg, MAX_REG) * CELL_IN_REG; cell++)
		for (uint8_t ic = 0; ic < total_ic; ic++)
			dest[ic][cell] = src[ic][cell];
}

uint8_t LTC6804_rdcv(uint8_t reg, uint8_t total_ic, uint16_t cell_codes[][12]) {
	if (!MockLTC::cellVoltageValid.check())
		return -1;

	read(reg, total_ic, 4, cell_codes, MockLTC::cellVoltages);
	return 0;
}

int8_t LTC6804_rdaux(uint8_t reg, uint8_t total_ic, uint16_t cell_codes[][6]) {
	if (!MockLTC::auxVoltageValid.check())
		return -1;

	read(reg, total_ic, 2, cell_codes, MockLTC::auxVoltages);
	return 0;
}
