#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class FCU_accelerometer_values {
public:
	FCU_accelerometer_values() = default;
	FCU_accelerometer_values(uint8_t buf[8]) { load(buf); }

	inline void load(uint8_t buf[]) 	{ memcpy(this, buf, sizeof(*this)); }
	inline void write(uint8_t buf[])	{ memcpy(buf, this, sizeof(*this)); }

	inline uint8_t get_x() const { return XValue_x100; }
	inline uint8_t get_y() const { return YValue_x100; }
	inline uint8_t get_z() const { return ZValue_x100; }

	inline void set_values(uint8_t x, uint8_t y, uint8_t z) {
		XValue_x100 = x;
		YValue_x100 = y;
		ZValue_x100 = z;
	}

private:
   	uint8_t XValue_x100;
   	uint8_t YValue_x100;
   	uint8_t ZValue_x100;
};

#pragma pack(pop)