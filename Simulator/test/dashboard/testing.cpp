#include "CAN_sim.h"
#include "Dashboard.h"
#include "DebouncedButton.h"
#include "gtest/gtest.h"
#include "HyTech_CAN.h"
#include "mcp_can.h"
#include "Simulator.h"
#include "VariableLED.h"

class DashboardTesting : public ::testing::Test {
protected:
	DashboardTesting () = default;
	void TearDown() {
		extern bool is_mc_err;
		extern VariableLED variable_led_start;
		extern Metro timer_can_update;
		extern MCP_CAN CAN;
		extern Dashboard_status dashboard_status;
		extern MCU_status mcu_status;

		CAN_simulator::purge();
		is_mc_err = false;
		variable_led_start = VariableLED(LED_START);
		timer_can_update = Metro(100);
		CAN = MCP_CAN(SPI_CS);
		dashboard_status = {};
		mcu_status = {};
	}	
};

TEST_F(DashboardTesting, First){
	Simulator simulator (100);
	CAN_message_t msg;

	delay(100);
	simulator.next();
	ASSERT_TRUE(CAN_simulator::get(msg));
	ASSERT_EQ(msg.id,ID_DASHBOARD_STATUS);
	Dashboard_status status;
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),0);
	EXPECT_EQ(status.get_led_flags(),3);
}

TEST_F(DashboardTesting, Second){
	Simulator simulator;
	CAN_message_t msg;

	delay(50);
	simulator.digitalWrite(23, 0);
	simulator.next();
	delay(100);
	simulator.next();
	ASSERT_TRUE(CAN_simulator::get(msg));
	ASSERT_EQ(msg.id,ID_DASHBOARD_STATUS);
	Dashboard_status status;
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),1);
	EXPECT_EQ(status.get_led_flags(),3);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}