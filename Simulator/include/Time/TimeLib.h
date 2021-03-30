#pragma once
#include <ctime>
#include <stdint.h>

typedef enum { timeNotSet, timeNeedsSync, timeSet } timeStatus_t;
typedef time_t(*getExternalTime)();

class teensy3_clock_class {
public:
	inline static time_t get() { return rtcTime; }
	inline static void set(time_t t) { if (valid) rtcTime = t; }
	inline static void teardown() { valid = rtcTime = 0; }

	friend uint16_t year();
	friend uint16_t month();
	friend uint16_t day();
	friend uint16_t hour();
	friend uint16_t minute();
	friend uint16_t second();
	friend void setSyncProvider(getExternalTime sync);

private:
	static time_t rtcTime;
	static bool valid;
};

extern teensy3_clock_class Teensy3Clock;

uint16_t year()		{ return localtime(&Teensy3Clock.rtcTime)->tm_year;	}
uint16_t month()	{ return localtime(&Teensy3Clock.rtcTime)->tm_mon;	}
uint16_t day()		{ return localtime(&Teensy3Clock.rtcTime)->tm_mday;	}
uint16_t hour()		{ return localtime(&Teensy3Clock.rtcTime)->tm_hour;	}
uint16_t minute()	{ return localtime(&Teensy3Clock.rtcTime)->tm_min;	}
uint16_t second()	{ return localtime(&Teensy3Clock.rtcTime)->tm_sec;	}

inline void setSyncProvider(getExternalTime sync) {
	Teensy3Clock.valid = (sync == teensy3_clock_class::get);
}

timeStatus_t timeStatus() {
	return Teensy3Clock.get() ? timeSet : timeNotSet;
}
