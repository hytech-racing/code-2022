#include "MockGPS.h"

#include "Adafruit_GPS.h"
#include "TimeLib.h"

#define GPRMC 1
#define GPGGA 3

void MockGPS::processString(const char* str) {
	if (strstr(str, "$PMTK314") == str) {
		str += strlen("$PMTK314");
		for (int i = 0; i < 6; ++i)
			bitWrite(send, i, str[2*i + 1] == '1');
	}
}

void MockGPS::transmit() {

	char buf [128];
	puts("SENDING");

	if (bitRead(send, GPGGA)) {
		char* p = buf;
		p += sprintf(p, "$GPGGA");
		p = addTime(p);
		p = addLat(p);
		p = addLon(p);
		p += sprintf(p, ",%d", quality);
		p += sprintf(p, ",%d", satellites);
		p += sprintf(p, ",%.1f", hdop);
		p += sprintf(p, ",%.1f,M", altitude);
		p += sprintf(p, ",%.1f,M", geoidHeight);
		p += sprintf(p, "*%02x", checksum(buf));
		puts(buf);
		serial->sim_println(buf);
	}
	if (bitRead(send, GPRMC)) {
		char* p = buf;
		p += sprintf(p, "$GPRMC");
		p = addTime(p);
		p += sprintf(p, ",%c", status_ok ? 'A' : 'V');
		p = addLat(p);
		p = addLon(p);
		p += sprintf(p, ",%.1f", speed);
		p += sprintf(p, ",%.1f", course);
		p += sprintf(p, ",%d", satellites);
		p += sprintf(p, ",%02d%02d%02d", day(), month(), year() % 100);
		p += sprintf(p, "*%02x", checksum(buf));
		puts(buf);
		serial->sim_println(buf);
	}
}

char* MockGPS::addTime(char* p) {
	p += sprintf(p, ",%02d", hour());
	p += sprintf(p, "%02d", minute());
	p += sprintf(p, "%02d", second());
	p += sprintf(p, ".%03d", (int) (millis() % 1000));
	return p;
}

char* MockGPS::addLat(char* p) {
	p += sprintf(p, ",%02d", latdeg);
	p += sprintf(p, "%02.4f", latmin * 60);
	p += sprintf(p, ",%c", latdir);
	return p;
}

char* MockGPS::addLon(char* p) {
	p += sprintf(p, ",%03d", londeg);
	p += sprintf(p, "%02.4f", lonmin * 60);
	p += sprintf(p, ",%c", londir);
	return p;
}

int MockGPS::checksum(char* sentence) {
	int cs = 0;
	for (++sentence; *sentence; ++sentence)
		cs ^= *sentence;
	return cs;
};