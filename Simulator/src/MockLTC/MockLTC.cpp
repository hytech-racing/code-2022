#include "MockLTC.h"
#include "LTC68042.h"

void MockLTC::processByte(byte received) {
	if (state == TRANSMITTING)
		write(*current);
	else
		*current = received;

	if (++current != end)
		return;

	if (current != checksum + 2) {
		current = checksum;
		end = checksum + 2;
	}
	else if (state == COMMAND)
		processCommand();
	else
		setState(COMMAND, cmd);
}

void MockLTC::processCommand() {
	if (pec15_calc(2, cmd) != ((checksum[0] << 8) | checksum[1]))
		throw LTCException("Invalid checksum for %x %x - expected %x but got %x%x", cmd[0], cmd[1], pec15_calc(2, cmd), checksum[0], checksum[1]);

	extern uint8_t ADCV[2]; //!< Cell Voltage conversion command.
	extern uint8_t ADAX[2]; //!< GPIO conversion command.

	if (cmd[0] == ADCV[0] && cmd[1] == ADCV[1])			{ setState(COMMAND, cmd); adcvReady = millis() + PROCESSING_TIME; }
	else if (cmd[0] == ADAX[0] && cmd[1] == ADAX[1])	{ setState(COMMAND, cmd); adaxReady = millis() + PROCESSING_TIME; }
	else if (cmd[0] == 0x07 && cmd[1] == 0x11)	{ setState(COMMAND, cmd); memset(cellVoltages, 0, TOTAL_IC * 12 * sizeof(uint16_t)); }
	else if (cmd[0] == 0x07 && cmd[1] == 0x12)	{ setState(COMMAND, cmd); memset(auxVoltages, 0, TOTAL_IC * 6 * sizeof(uint16_t)); }

	else {
		int ic = (cmd[0] - 0x80) >> 3;
		if (ic >= TOTAL_IC)
			throw LTCException("Invalid IC ID (max %d , got %d)", TOTAL_IC - 1, ic);
		if (cmd[1] >= 0x0F)
			throw LTCException("Invalid command %x %x", cmd[0], cmd[1]);
		if (cmd[1] == 0x01)
			setState(RECEIVING, cfg[ic]);
		else if (cmd[1] == 0x02)
			setState(TRANSMITTING, cfg[ic]);
		else if (cmd[1] <= 0x0A) {
			if (adcvReady < millis())
				throw LTCException("Cell voltage calculations not ready.");
			setState(TRANSMITTING, (uint8_t*) cellVoltages[ic] + (cmd[1] - 0x04) * 3);
		}
		else if (cmd[1] <= 0x0E) {
			if (adaxReady < millis())
				throw LTCException("Aux voltage calculations not ready.");
			setState(TRANSMITTING, (uint8_t*) auxVoltages[ic] + (cmd[1] - 0x0C) * 3);
		}
	}
}

void MockLTC::setState(State newState, uint8_t* newCurrent) {
	state = newState;
	current = newCurrent;
	end = current + (state == COMMAND ? 2 : 6);
	if (state == TRANSMITTING)
		* (uint16_t*) checksum = pec15_calc(6,current);
}
