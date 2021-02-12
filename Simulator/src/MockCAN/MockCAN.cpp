#include "MockCAN.h"

#include <queue>

std::queue<CAN_message_t> MockCAN::vehicle_inbox;
std::queue<CAN_message_t> MockCAN::vehicle_outbox;

int MockCAN::read(CAN_message_t &msg, std::queue<CAN_message_t>& mailbox) {
	if (mailbox.empty())
		return false;
	msg = mailbox.front();
	mailbox.pop();
	return true;
}