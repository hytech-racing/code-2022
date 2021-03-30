#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass @ID(ID_FCU_ACCELEROMETER)
class FCU_accelerometer_values {
public:
	FCU_accelerometer_values() = default;
	FCU_accelerometer_values(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

	inline uint8_t get_x() const { return XValue_x100; }
	inline uint8_t get_y() const { return YValue_x100; }
	inline uint8_t get_z() const { return ZValue_x100; }

	inline void set_values(uint8_t x, uint8_t y, uint8_t z) {
		XValue_x100 = x;
		YValue_x100 = y;
		ZValue_x100 = z;
	}

#ifdef HT_DEBUG_EN
    void print() {
		Serial.println("\n\nACCELEROMETER DATA");
		Serial.println(    "------------------");
		Serial.print("X:"); Serial.println(XValue_x100 / 100.0, 2);
		Serial.print("Y:"); Serial.println(YValue_x100 / 100.0, 2);
		Serial.print("Z:"); Serial.println(ZValue_x100 / 100.0, 2);
	}
#endif

private:
   	uint8_t XValue_x100; // @Parse @Name(accel_x) @Getter(get_x) @Scale(100) @Unit(m/s^2)
   	uint8_t YValue_x100; // @Parse @Name(accel_y) @Getter(get_y) @Scale(100) @Unit(m/s^2)
   	uint8_t ZValue_x100; // @Parse @Name(accel_z) @Getter(get_z) @Scale(100) @Unit(m/s^2)
};

#pragma pack(pop)