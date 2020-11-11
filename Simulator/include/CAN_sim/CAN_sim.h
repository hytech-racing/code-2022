#pragma once

#include <queue>

#include "HyTech_FlexCAN.h"

namespace CAN_simulator {
	int push(const CAN_message_t& msg);
	int get(CAN_message_t &msg);
	int sim_read(CAN_message_t &msg);
	void purge();
	extern std::queue<CAN_message_t> inbox;
	extern std::queue<CAN_message_t> outbox;
};

