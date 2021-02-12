#include "MockGPS.h"
#include "TimeLib.h"

void MockGPS::processSerial(char* str) {
	if (strstr(str, "$PMTK314") == str)
		for (int i = 0; i < 6; ++i)
			bitSet(send, i, str[strlen("$PMTK314") + 2 * (i + 1) == '1']);
}

void MockGPS::transmit() {

	char buff [128];
	char* p = buff;

	if (bitRead(send, GPGGA)) {
		p = sprintf(p, "$GPGGA");
		p = addTime(p);
		p = addLat(p);
		p = addLon(p);
		p = sprintf(p, ",%d", quality);
		p = sprintf(p, ",%d", satellites);
		p = sprintf(p, ",%.1f", hdop);
		p = sprintf(p, ",%.1f,M", altitude);
		p = sprintf(p, ",%.1f,M", geoidHeight);
		p = sprintf(p, "*%02x", checksum(buff));
		serial.sim_println(buff);
	}
	if (bitRead(send, GPRMC)) {
		p = sprintf(p, "$GPRMC");
		p = addTime(p);
		p = sprintf(p, ",%c", status);
		p = addLat(p);
		p = addLon(p);
		p = sprintf(p, ",%.1f", speed);
		p = sprintf(p, ",%.1f", course);
		p = sprintf(p, ",%d", satellites);
		p = sprintf(p, ",%02d%02d%02d", date(), month(), year() % 100);
		p = sprintf(p, "*%02x", checksum(buff));
		serial.sim_println(buff);
	}
}

char* MockGPS::addTime(char* p) {
	p = sprintf(p, ",%02d", hour());
	p = sprintf(p, "%02d", minute());
	p = sprintf(p, "%02d", second());
	p = sprintf(p, ".%03d", millis() % 1000);
	return p;
}

char* MockGPS::addLat(char* p) {
	p = sprintf(p, ",%02d", latdeg);
	p = sprintf(p, "%02.4f", latmin * 60);
	p = sprintf(p, ",%c", latdir);
	return p;
}

char* MockGPS::addLon(char* p) {
	p = sprintf(p, ",%03d", londeg);
	p = sprintf(p, "%02.4f", lonmin * 60);
	p = sprintf(p, ",%c", londir);
	return p;
}

int MockGPS::checksum(char* sentence) {
	int cs = 0;
	for (++sentence; *sentence; ++sentence)
		cs ^= *sentence;
	return cs;
}