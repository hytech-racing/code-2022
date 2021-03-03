#pragma once

#include "I2CListener.h"
#include "Adafruit_ADXL345_U.h"

class MockADXL : public I2CListener {
public:
	MockADXL() : I2CListener(ADXL345_ADDRESS) {}

	inline void begin() {
		I2CListener::begin();
		memset(registers, 0, sizeof(registers));

		registers[ADXL345_REG_DEVID] = 0xE5;
		registers[ADXL345_REG_BW_RATE] = ADXL345_DATARATE_0_10_HZ;
		registers[ADXL345_REG_DATA_FORMAT] = ADXL345_RANGE_2_G;
	}

	inline int transmit(uint8_t* buffer, size_t maxlen, bool sendStop) {
		if (regToSend == -1 || regToSend + maxlen >= sizeof(registers))
			return -1;
		for (int i = 0; i < maxlen; ++i)
			buffer[i] = registers[regToSend + i];
		regToSend = -1;
		return 0;
	}

	inline int8_t process(uint8_t* buffer, size_t len, bool sendStop) {
		if (len == 1)			regToSend = buffer[0];
		else if (len == 2)		registers[buffer[0]] = buffer[1];
		return 0;
	};

	inline byte getRegister (byte reg)				{ return registers[reg]; }
	inline void setRegister (byte reg, byte value) 	{ registers[reg] = value; }
	inline void setValues (float x, float y, float z) { setX(x); setY(y); setZ(z); }
	inline void setX (float x) { set16(ADXL345_REG_DATAX0, x); };
	inline void setY (float y) { set16(ADXL345_REG_DATAY0, y); };
	inline void setZ (float z) { set16(ADXL345_REG_DATAZ0, z); };
private:
	inline void set16(byte reg, float value) {
		int16_t scaled = value / (ADXL345_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD);

		registers[reg] = scaled & 0xFF;
		registers[reg + 1] = scaled >> 8;
	}

	byte registers [64];
	int regToSend = -1;
};
    