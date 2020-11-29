#include "CAN_simulator.h"

namespace CAN_simulator {
	std::queue<CAN_message_t> vehicle_inbox;
	std::queue<CAN_message_t> vehicle_outbox;

	namespace {
		int read_helper(CAN_message_t &msg, std::queue<CAN_message_t>& mailbox) {
			if (mailbox.empty())
				return false;
			msg = mailbox.front();
			mailbox.pop();
			return true;
		}

		int write_helper(const CAN_message_t& msg, std::queue<CAN_message_t>& mailbox) {
			mailbox.push(msg);
			return true;
		}
	}

	int read(CAN_message_t &msg) { 
		return read_helper(msg, vehicle_outbox);
	}

	int write(const CAN_message_t& msg) {
		return write_helper(msg, vehicle_inbox);
	}

	int vehicle_read(CAN_message_t &msg) {
		return read_helper(msg, vehicle_inbox);
	}

	int vehicle_write(const CAN_message_t &msg) {
		return write_helper(msg, vehicle_outbox);
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
