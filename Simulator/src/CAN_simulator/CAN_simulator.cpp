#include "CAN_simulator.h"

namespace CAN_simulator {
	std::queue<CAN_message_t> vehicle_inbox;
	std::queue<CAN_message_t> vehicle_outbox;

	int write(const CAN_message_t& msg) { vehicle_inbox.push(msg); return true; }

	int read(CAN_message_t &msg) { 
		if (vehicle_outbox.empty())
			return false;
		msg = vehicle_outbox.front(); vehicle_outbox.pop();
		return true;
	}

	int vehicle_read(CAN_message_t &msg) {
		if (vehicle_inbox.empty())
			return false;
		msg = vehicle_inbox.front();
		vehicle_inbox.pop();
		return true;
	}

	void purge() {
		vehicle_inbox = std::queue<CAN_message_t>();
		vehicle_outbox = std::queue<CAN_message_t>();

		#ifdef HYTECH_ARDUINO_TEENSY_32
			FLEXCAN0_IMASK1 = 0;
		#elif defined (HYTECH_ARDUINO_TEENSY_35)
			FLEXCAN0_MCR = 0xFFFFFFFF;
		#endif
	}
}
