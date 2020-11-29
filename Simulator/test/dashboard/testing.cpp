#include "CAN_simulator.h"
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

//Startup Test
TEST_F(DashboardTesting, Startup){
	Simulator simulator (100);
	CAN_message_t msg;

	simulator.next();
	ASSERT_TRUE(CAN_simulator::read(msg));
	ASSERT_EQ(msg.id,ID_DASHBOARD_STATUS);
	Dashboard_status status;
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),0);
	EXPECT_EQ(status.get_led_flags(),3); //On startup all MC Status flags are 0, which cause the AMS and IMD LED flags to go to 1 which sets the flags to 00000011
}


//Indiviual Button Tests.  Press, wait 50 ms, depress, wait 51 ms, press, wait 50 ms, depress
TEST_F(DashboardTesting, MarkButton){
	Simulator simulator;
	CAN_message_t msg;
	Dashboard_status status;

	simulator.digitalWrite(BTN_MARK, 0);
	simulator.next();
	delay(50);
	simulator.digitalWrite(BTN_MARK, 1);
	simulator.next();
	delay(51);
	simulator.next();

	CAN_simulator::read(msg);
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),1);
	EXPECT_EQ(status.get_led_flags(),3);
	simulator.digitalWrite(BTN_MARK, 0);

	simulator.next();
	delay(50);
	simulator.next();
	delay(50);
	simulator.next();

	CAN_simulator::read(msg);
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),0);
	EXPECT_EQ(status.get_led_flags(),3);
}


//LED Tests
TEST_F(DashboardTesting, LEDTest){
	Simulator simulator;
	CAN_message_t msg;
	Dashboard_status status;
	MCU_status mcu_stat;
	byte mcu_msg[8];


	//Nothing should happen to LEDs on Startup
	CAN_simulator::read(msg);
	status.load(msg.buf);
	EXPECT_EQ(status.get_led_flags(),3);

	//set set_bms_ok_high to true and push message
	mcu_stat.set_bms_ok_high(true);
	mcu_stat.write(msg.buf);
	msg.id = ID_MCU_STATUS;
	msg.len = sizeof(mcu_stat);

	//CAN_simulator::push(msg);
	//how do you send this message?

	simulator.next();
	delay(50);
	simulator.next();
	delay(50);
	simulator.next();

	CAN_simulator::read(msg);
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),0);
	EXPECT_EQ(status.get_led_flags(),2);
}

//Example Test
TEST_F(DashboardTesting, Third){
	Simulator simulator;
	CAN_message_t msg;

	delay(50);
	simulator.digitalWrite(23, 0);
	simulator.next();
	delay(100);
	simulator.next();
	ASSERT_TRUE(CAN_simulator::read(msg));
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