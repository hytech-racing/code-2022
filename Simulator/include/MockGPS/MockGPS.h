#pragma once

#include "Adafruit_GPS.h"

class MockGPS : public SerialListener {
public:
	void processSerial(char* str);
	void begin();
	void transmit();

	inline void setQuality(int q)			{ quality = q; }
	inline void setSatCount(int s)			{ satellites = s; }
	inline void setHDOP(int h)				{ hdop = h; }
	inline void setAltitude(float a)		{ alt = a; }
	inline void setGeoidHeight(float g)		{ geoid = g; }
	inline void setSpeed(float s)			{ speed = s; }
	inline void setCourse(float c)			{ course = c; }
	inline void setStatus(char c)			{ course = c; }
	inline void setLat(float deg) {
		latdir = deg < 0 ? 'S' : 'N';
		deg = abs(deg);
		latdeg = (int) deg;
		latmin = deg - latdeg;
	}
	inline void setLon(float deg) {
		londir = deg < 0 ? 'W' : 'E';
		deg = abs(deg);
		londeg = (int) deg;
		lonmin = deg - londeg;
	}

private:
	char* addTime(char*);
	char* addLat(char*);
	char* addLon(char*);
	int* checksum(char*);

	int send;
	int quality, satellites;
	float hdop, alt, geoid, speed, course;
	char status;

	int latdeg, londeg;
	float latmin, lonmin;
	char latdir, londir;
}

