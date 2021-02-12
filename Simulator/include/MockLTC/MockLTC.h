#include <LTC68042.h>

#include "SPIListener.h"

#define PROCESSING_TIME 202

class MockLTC : public SPIListener {
	typedef enum { COMMAND, TRANSMITTING, RECEIVING } State;

public:
	MockLTC(int cs, int ic) : SPIListener(cs), TOTAL_IC(ic) {
		cellVoltages = new uint16_t [ic][12];
		auxVoltages = new uint16_t [ic][6];
		cfg = new uint8_t[ic][6];
		state = COMMAND;
		current = cmd, end = cmd + 2;
	}

	inline void teardown() {
		delete [] cellVoltages;
		delete [] auxVoltages;
		delete [] cfg;
	}

	void processByte(byte received);
	void processCommand();

	inline void setCellVoltage(int ic, int cell, uint16_t value)	{ cellVoltages[ic][cell] = value; }
	inline void setAuxVoltage(int ic, int cell, uint16_t value) 	{ auxVoltages[ic][cell] = value; }
	inline void setConfig(uint8_t* newConfig)	{ memcpy(cfg, newConfig, 6); }
	inline void getConfig(uint8_t* dest)		{ memcpy(dest, cfg, 6); }

private:
	State state;
	uint8_t *current, *end;
	uint16_t (*cellVoltages)[12], (*auxVoltages)[6];
	uint8_t (*cfg) [6];
	uint8_t cmd [2], checksum [2];
	const int TOTAL_IC;
	unsigned long long adcvReady, adaxReady;
};