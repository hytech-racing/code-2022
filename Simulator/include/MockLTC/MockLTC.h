#pragma once

#include <stdint.h>
#include <queue>

typedef struct ADC_SETTINGS { uint8_t MD, DCP, CH, CHG; } ADC_SETTINGS;

namespace MockLTC {
	extern std::queue<uint8_t(*)[6]> cfg_list;
	extern ADC_SETTINGS adc;

	void init (int ic);
	void setCellVoltages(uint16_t(*voltages)[12]);
	void setAuxVoltages(uint16_t(*voltages)[6]);
	void teardown();
};