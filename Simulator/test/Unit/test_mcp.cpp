#include <time.h>

#include "gtest/gtest.h"
#include "MockGPS.h"
#include "MockMCP.h"
#include "Simulator.h"

TEST(UNIT, MockMCP) {
	srand(time(0));
	MCP23S17 real_mcp(0, 10, 9000000);
	MockMCP mock_mcp(0, 10);

	mock_mcp.begin();
	Simulator::begin();
	real_mcp.begin();

	uint16_t random = rand() & 0xFFFF;
	for (int i = 0; i < 16; ++i) {
		real_mcp.pinMode(i, INPUT);
		mock_mcp.setGPIOValue(i, (random >> i) & 1);
	}
	ASSERT_EQ(random, real_mcp.digitalRead());

	for (int i = 0; i < 16; ++i) {
		real_mcp.pullupMode(i, HIGH);
		mock_mcp.setGPIOUnknown(i, true);
	}
	ASSERT_EQ(0xFFFF, real_mcp.digitalRead());

	for (int i = 0; i < 16; ++i)
		real_mcp.inputInvert(i, HIGH);
	ASSERT_EQ(0, real_mcp.digitalRead());

	random = rand() & 0xFFFF;
	for (int i = 0; i < 16; ++i)
		real_mcp.pinMode(i, OUTPUT);
	real_mcp.digitalWrite(random);

	for (int i = 0; i < 16; ++i)
		ASSERT_EQ((random >> i) & 1, mock_mcp.getOLATValue(i));

	mock_mcp.teardown();
	Simulator::teardown();
}