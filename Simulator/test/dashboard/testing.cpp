#include "gtest/gtest.h"

#include "Simulator.h"
#include "CAN_sim.h"

TEST(First, First){
    Simulator simulator (100);
    CAN_message_t msg;

    msg.id = 0x1;
    msg.len = 8;
    msg.buf[0] = 'A';
    msg.buf[1] = 'B';
    msg.buf[2] = 'C';
    msg.buf[3] = 'D';
    msg.buf[4] = 'E';
    msg.buf[5] = 'F';
    msg.buf[6] = 'G';
    msg.buf[7] = '\0';
    CAN_simulator::push(msg);
    msg.buf[0] = 'H';
    CAN_simulator::push(msg);

    simulator.next();
    ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}