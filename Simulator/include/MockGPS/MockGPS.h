#pragma once

#include "Adafruit_GPS.h"

class MockGPS : public SerialListener {
public:
	MockGPS(HardwareSerial* serial) : SerialListener(serial) {}
	void processString(const char* str);
	void transmit();

	inline void setQuality(int q)			{ quality = q; }
	inline void setSatCount(int s)			{ satellites = s; }
	inline void setHDOP(int h)				{ hdop = h; }
	inline void setAltitude(float a)		{ altitude = a; }
	inline void setGeoidHeight(float g)		{ geoidHeight= g; }
	inline void setSpeed(float s)			{ speed = s; }
	inline void setCourse(float c)			{ course = c; }
	inline void setStatus(bool ok)			{ status_ok = ok; }
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

	inline void setCoordinates(float lat, float lon) { setLat(lat); setLon(lon); }
	inline void transmit(float lat, float lon) { setCoordinates(lat, lon); transmit(); }

private:
	char* addTime(char*);
	char* addLat(char*);
	char* addLon(char*);
	int checksum(char*);

	int send;
	int quality, satellites;
	float hdop, altitude, geoidHeight, speed, course;
	bool status_ok;

	int latdeg, londeg;
	float latmin, lonmin;
	char latdir, londir;
};

