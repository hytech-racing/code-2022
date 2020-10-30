#pragma once
#include <HyTech_FlexCAN/HyTech_FlexCAN.h>
#include <queue>

namespace CAN_simulator {
	int push(const CAN_message_t& msg);
	int get(CAN_message_t &msg);
	int sim_read(CAN_message_t &msg);
	extern std::queue<CAN_message_t> inbox;
	extern std::queue<CAN_message_t> outbox;
};

