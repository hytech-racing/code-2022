#include "CAN_sim.h"

namespace CAN_simulator {
	std::queue<CAN_message_t> inbox;
	std::queue<CAN_message_t> outbox;

	int push(const CAN_message_t& msg) { inbox.push(msg); return true; }

	int get(CAN_message_t &msg) { 
		if (outbox.empty())
			return false;
		msg = outbox.front(); outbox.pop();
		return true;
	}

	int sim_read(CAN_message_t &msg) {
		if (inbox.empty())
			return false;
		msg = inbox.front();
		inbox.pop();
		return true;
	}
}
