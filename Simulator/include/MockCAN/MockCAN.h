#pragma once

#include "HyTech_FlexCAN.h"

namespace MockCAN {
	int read(CAN_message_t &msg);
	int write(const CAN_message_t& msg);
	int vehicle_read(CAN_message_t &msg);
	int vehicle_write(const CAN_message_t &msg);
	size_t vehicle_avail();
	void purge();
};

