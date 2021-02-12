#pragma once

#include <unordered_map>

#include "Arduino.h"
#include "Listener.h"
#include "SPI.h"

#define LSBFIRST 0
#define MSBFIRST 1

class SPIListener : public Listener {
public:
	SPIListener(int cs, int mode = MSBFIRST) : cs(cs), mode(mode) {}

	void begin() { 
		Simulator::attachListener(CS, this); 
		Simulator::attachListener(MOSI, this); 
		if (!listeners.insert(std::pair<uint8_t, SPIListener*>(cs, this)).second)
			throw SPIException("This may occur if you try to register two listeners at the same CS (CS = %d)", cs);
	}

	inline void teardown() {
		listeners.erase(cs);
		if (selected == this) selected = nullptr;
	}

	inline static void static_teardown() { 
		listeners.clear();
		selected = nullptr;
	}

	void process(uint8_t pinNo, unsigned reading);

protected:
	inline void write(int val) { response = val; }
	virtual void processByte(byte) = 0;

private:
	int cs, mode, spiCount;
	byte response = 0;
	static std::unordered_map<uint8_t,SPIListener*> listeners;
	static SPIListener* selected;
	friend struct SPIUtils::VolatileRegister;
};