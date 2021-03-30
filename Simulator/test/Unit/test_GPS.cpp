#include <time.h>

#include "gtest/gtest.h"
#include "MockGPS.h"
#include "Adafruit_GPS.h"
#include "Simulator.h"

TEST(UNIT, MockGPS) {
	srand(time(0));
	Adafruit_GPS real_gps(&Serial1);
	MockGPS mock_gps = MockGPS(&Serial1);

	mock_gps.begin();
	Simulator::begin();

    real_gps.begin(9600);
    real_gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    real_gps.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
    real_gps.sendCommand(PGCMD_ANTENNA);

	float lat = (rand() % 18000) / 100.0 - 90;
	float lon = (rand() % 36000) / 100.0 - 180;
	mock_gps.transmit(lat, lon);

	int messages = 0;
	while (real_gps.read()) {
		if (real_gps.newNMEAreceived()) {
			++messages;
			real_gps.parse(real_gps.lastNMEA());
			ASSERT_FLOAT_EQ(lat, real_gps.latitudeDegrees);
			ASSERT_FLOAT_EQ(lon, real_gps.longitudeDegrees);
		}
	}
	ASSERT_EQ(messages, 2);

	mock_gps.teardown();
	Simulator::teardown();
}