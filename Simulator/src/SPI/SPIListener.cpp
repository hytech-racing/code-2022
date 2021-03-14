#include "Arduino.h"
#include "SPIListener.h"

std::unordered_map<uint8_t,SPIListener*> SPIListener::listeners; 
SPIListener* SPIListener::selected = nullptr;

void SPIListener::process(uint8_t pinNo, unsigned reading) {
	if (pinNo == cs) {
		if (!selected && reading == LOW)
			selected = this;
		else if (selected == this && reading == HIGH)
			selected = nullptr;
		return;
	}

	static uint8_t input = 0, inputMask = mode == MSBFIRST ? 0x80 : 0x00;
	if (Simulator::digitalRead(cs) == HIGH)
		return;

	if (Simulator::digitalRead(MOSI))
		input |= inputMask;	
	inputMask >>= mode == MSBFIRST ? 1 : -1;

	if (inputMask == 0) {
		processByte(input);
		inputMask = mode == MSBFIRST ? 0x80 : 0x00;
		input = 0;
	}

	Simulator::digitalWrite(MISO, response & inputMask);
}