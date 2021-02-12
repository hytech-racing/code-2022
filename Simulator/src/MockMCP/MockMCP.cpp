#include "MockMCP.h"

void MockMCP::processByte(byte value) {
	if (state == IDLE) {
		opcode = value & (OPCODE_WRITE | OPCODE_READ);
		if ((value ^ opcode) != address)
			return;
	}
	else if (state == SELECT_REGISTER)
		targetRegister = value;
	else if (opcode == OPCODE_WRITE)
		values[targetRegister] = value;
	else if (targetRegister == GPIOA)
		write((values[targetRegister] & values[IODIRA] | (values[GPPUA] & unknown[UNKWNA])) ^ values[IPOLA]);
	else if (targetRegister == GPIOB)
		write((values[targetRegister] & values[IODIRB] | (values[GPPUB] & unknown[UNKWNB])) ^ values[IPOLB]);
	else
		write(values[targetRegister]);

	state = (State) ((state + 1) % (READ_OR_TRANSMIT + 1));
}
