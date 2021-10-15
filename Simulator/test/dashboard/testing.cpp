#include "MockCAN.h"
#include "Dashboard.h"
#include "DebouncedButton.h"
#include "gtest/gtest.h"
#include "HyTech_CAN.h"
#include "mcp_can.h"
#include "Simulator.h"
#include "VariableLED.h"

#define next() Simulator::next()
#define digitalWrite(pin,val) Simulator::digitalWrite(pin, val);

class DashboardTesting : public ::testing::Test {
protected:
	DashboardTesting () {}
	void SetUp() {
		extern VariableLED led_start;
		extern Metro timer_can_update;
		extern MCP_CAN CAN;
		extern Dashboard_status dashboard_status;
		extern MCU_status mcu_status;

		MockCAN::teardown();
		led_start = VariableLED(LED_START);
		timer_can_update = Metro(100);
		CAN = MCP_CAN(SPI_CS);
		dashboard_status = {};
		mcu_status = {};
		Simulator::begin();
	}

	void TearDown() {
		Simulator::teardown();
	}
};

//Startup Test
TEST_F(DashboardTesting, Startup){
	CAN_message_t msg;
	delay(100);
	next();
	ASSERT_TRUE(MockCAN::read(msg));
	ASSERT_EQ(msg.id,ID_DASHBOARD_STATUS);
	Dashboard_status status;
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),0);
	EXPECT_EQ(status.get_led_flags(),3); //On startup all MC Status flags are 0, which cause the AMS and IMD LED flags to go to 1 which sets the flags to 00000011
}


//Indiviual Button Tests.  Press, wait 50 ms, depress, wait 51 ms, press, wait 50 ms, depress
TEST_F(DashboardTesting, MarkButton){
	CAN_message_t msg;
	Dashboard_status status;

	digitalWrite(BTN_MARK, LOW);
	next();
	delay(50);
	digitalWrite(BTN_MARK, HIGH);
	next();
	delay(51);
	next();

	MockCAN::read(msg);
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),1);
	EXPECT_EQ(status.get_led_flags(),3);
	digitalWrite(BTN_MARK, LOW);

	next();
	delay(50);
	next();
	delay(50);
	next();

	MockCAN::read(msg);
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),0);
	EXPECT_EQ(status.get_led_flags(),3);
}


//LED Tests
TEST_F(DashboardTesting, LEDTest){
	CAN_message_t msg;
	Dashboard_status status;
	MCU_status mcu_stat;
	byte mcu_msg[8];


	//Nothing should happen to LEDs on Startup
	MockCAN::read(msg);
	status.load(msg.buf);
	EXPECT_EQ(status.get_led_flags(),3);

	//set set_bms_ok_high to true and push message
	mcu_stat.set_bms_ok_high(true);
	mcu_stat.write(msg.buf);
	msg.id = ID_MCU_STATUS;
	msg.len = sizeof(mcu_stat);

	//MockCAN::push(msg);
	//how do you send this message?

	next();
	delay(50);
	next();
	delay(50);
	next();

	MockCAN::read(msg);
	status.load(msg.buf);
	EXPECT_EQ(status.get_button_flags(),0);
	EXPECT_EQ(status.get_led_flags(),2);
}

//Example Test
TEST_F(DashboardTesting, Third){
	CAN_message_t msg;

	delay(50);
	digitalWrite(BTN_MARK, LOW);
	next();
	delay(100);
	next();
	ASSERT_TRUE(MockCAN::read(msg));
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