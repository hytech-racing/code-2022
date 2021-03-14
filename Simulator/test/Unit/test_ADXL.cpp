#include <time.h>

#include "gtest/gtest.h"
#include "MockADXL.h"
#include "Adafruit_ADXL345_U.h"
#include "Simulator.h"

TEST(UNIT, MockADXL) {
	srand(time(0));
	Adafruit_ADXL345_Unified real_accel = Adafruit_ADXL345_Unified(12345);
	MockADXL mock_accel = MockADXL();

	mock_accel.begin();
	Simulator::begin();

	ASSERT_TRUE(real_accel.begin());
	real_accel.setRange(ADXL345_RANGE_4_G);
	ASSERT_EQ(mock_accel.getRegister(ADXL345_REG_DATA_FORMAT) & 0x3, ADXL345_RANGE_4_G);

	float x = (rand() & 0xFFF) * ADXL345_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
	float y = (rand() & 0xFFF) * ADXL345_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
	float z = (rand() & 0xFFF) * ADXL345_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;

	mock_accel.setValues(x,y,z);

    sensors_event_t event;
    real_accel.getEvent(&event);

	constexpr float EPSILON = 0.1;
	ASSERT_LT(abs(x - event.acceleration.x), EPSILON);
	ASSERT_LT(abs(y - event.acceleration.y), EPSILON);
	ASSERT_LT(abs(z - event.acceleration.z), EPSILON);

	mock_accel.teardown();
	Simulator::teardown();
}