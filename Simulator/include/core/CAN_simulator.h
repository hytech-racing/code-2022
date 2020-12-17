#pragma once

#include <queue>

#include "HyTech_FlexCAN.h"

namespace CAN_simulator {
	int read(CAN_message_t &msg);
	int write(const CAN_message_t& msg);
	int vehicle_read(CAN_message_t &msg);
	int vehicle_write(const CAN_message_t &msg);
	void purge();
	extern std::queue<CAN_message_t> vehicle_inbox;
	extern std::queue<CAN_message_t> vehicle_outbox;
};

