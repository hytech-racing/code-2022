#include "gtest/gtest.h"
#include "HyTech_CAN.h"
#include "Simulator.h"
#include "CAN_sim.h"

TEST(First, First){
    Simulator simulator (100);
    CAN_message_t msg;

    delay(100);
    simulator.next();
    ASSERT_TRUE(CAN_simulator::get(msg));
    ASSERT_EQ(msg.id,ID_DASHBOARD_STATUS);
    Dashboard_status status;
    status.load(msg.buf);
    ASSERT_EQ(status.get_button_flags(),0);
    ASSERT_EQ(status.get_led_flags(),0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}