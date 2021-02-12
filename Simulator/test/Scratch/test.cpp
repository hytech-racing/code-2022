#include "gtest/gtest.h"

#include "MockADXL.h"
#include "Simulator.h"
#include "MockGPS.h"

MockADXL mockADXL;

class Testing : public ::testing::Test {
protected:
	void SetUp() {
		// mockMCP.begin();
		mockADXL.begin();
		Simulator::begin();
	}

	void TearDown() {
		Simulator::teardown();
		mockADXL.teardown();
	}
};

TEST_F(Testing, Test){
	mockADXL.setValues(420, 69, 101);
	Simulator::next();

	// for (int i = 0; i < 16; ++i) {
	// 	bool x = rand() & 1;
	// 	printf("GPIO SET %d: %d\n", i, x);
	// 	mockMCP.setGPIOValue(i, x);
	// }
	// mockadc.set(5, 5);
	// Simulator::next();
	// for (int i = 0; i < 16; ++i)
	// 	printf("OLAT GET %d: %d\n", i, mockMCP.getOLATValue(i));
}

