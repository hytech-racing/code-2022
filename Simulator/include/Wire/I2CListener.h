#pragma once

#include "HTException.h"
#include "Listener.h"
#include "Wire.h"

class I2CListener : public Listener { 
public:
	I2CListener(uint8_t address) : address(address) {}

	inline virtual void begin() { 
		if (!listeners.insert(std::pair<uint8_t, I2CListener*>(address, this)).second)
			throw I2CException("This may occur if you try to register two listeners at the same address (address = %d)", address);
	}

	inline void static_teardown() { listeners.clear(); };
	inline void teardown() { listeners.erase(address); }

	inline virtual void process(uint8_t pinNo, unsigned value) {
		throw I2CException("I2C Listeners should not attempt to listen to any physical pins. Use process(uint8_t* data, size_t len, bool sendStop).");
	}

	virtual int transmit(uint8_t* buffer, size_t maxlen, bool sendStop) = 0;
	virtual int8_t process(uint8_t* buffer, size_t len, bool sendStop) = 0;

private:
	uint8_t address;

// static functionality
	static std::unordered_map<uint8_t,I2CListener*> listeners;
	friend int twi_readFrom(uint8_t address, uint8_t* buffer, size_t maxlen, bool sendStop);
	friend int8_t twi_writeTo(uint8_t address, uint8_t* buffer, size_t len, bool sendStop);

	inline static I2CListener* getListener(uint8_t address) {
		I2CListener* listener = I2CListener::listeners[address];
		if (!listener)
			throw I2CException("Unable to find Mock I2C Entity with address %d.", address);
		return listener;
	}
};

inline int twi_readFrom(uint8_t address, uint8_t* buffer, size_t maxlen, bool sendStop) {
	return I2CListener::getListener(address)->transmit(buffer, maxlen, sendStop);
}

inline int8_t twi_writeTo(uint8_t address, uint8_t* buffer, size_t len, bool sendStop) {
	I2CListener::getListener(address)->process(buffer, len, sendStop);
}