#include "MockLTC.h"

void MockLTC::processByte(byte received) {
	if (state == RECEIVING)
		*current = received;
	else
		write(*current);

	if (++current != end)
		return;

	if (current != checksum + 2) {
		current = checksum;
		end = checksum + 2;
	}
	else if (state == COMMAND)
		processCommand();
	else
		state = COMMAND;
}

void MockLTC::processCommand() {
	if (pec15_calc(2, cmd) != * (uint16_t*) checksum)
		throw LTCException("Invalid checksum %x %x %x %x", cmd[0], cmd[1], checksum[0], checksum[1]);

	if (cmd[0] == 0x03 && cmd[1] == 0xD0)		adcvReady = millis() + PROCESSING_TIME;
	else if (cmd[0] == 0x05 && cmd[1] == 0xD0)	adaxReady = millis() + PROCESSING_TIME;
	else if (cmd[0] == 0x07 && cmd[1] == 0x11)	memset(cellVoltages, 0, TOTAL_IC * 14 * sizeof(uint16_t));
	else if (cmd[0] == 0x07 && cmd[1] == 0x12)	memset(auxVoltages, 0, TOTAL_IC * 8 * sizeof(uint16_t));

	else {
		int ic = (cmd[0] - 0x80) >> 3;
		if (ic >= TOTAL_IC)
			throw LTCException("Invalid IC ID (max %d , got %d)", TOTAL_IC - 1, ic);
		if (cmd[1] >= 0x0F)
			throw LTCException("Invalid command %x %x", cmd[0], cmd[1]);
		if (cmd[1] == 0x01) {
			state = RECEIVING;
			current = cfg[ic];
			return;
		}

		if (cmd[1] == 0x02)
			current = cfg[ic];
		else if (cmd[1] <= 0x0A) {
			if (adcvReady < millis())
				throw LTCException("Cell voltage calculations not ready.");
			current = (uint8_t*) cellVoltages[ic] + cmd[1] - 2;
		}
		else if (cmd[1] <= 0x0E) {
			if (adaxReady < millis())
				throw LTCException("Aux voltage calculations not ready.");
			current = (uint8_t*) auxVoltages[ic] + cmd[1] - 6;
		}

		state = TRANSMITTING;
		end = current + 6;
		* (uint16_t*) checksum = pec15_calc(6,current);
	}
}

