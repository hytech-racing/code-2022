#include "Wire.h"

#include "Arduino.h"
#include "I2CListener.h"

std::unordered_map<uint8_t, I2CListener*> I2CListener::listeners;

uint8_t TwoWire::rxBuffer[BUFFER_LENGTH];
uint8_t TwoWire::rxBufferIndex = 0;
uint8_t TwoWire::rxBufferLength = 0;

uint8_t TwoWire::txAddress = 0;
uint8_t TwoWire::txBuffer[BUFFER_LENGTH];
uint8_t TwoWire::txBufferIndex = 0;

uint8_t TwoWire::transmitting = 0;

static int default_sda_pin = SDA;
static int default_scl_pin = SCL;

TwoWire::TwoWire() {}

void TwoWire::begin(void) {
	begin(default_sda_pin, default_scl_pin);
}

void TwoWire::begin(int sda, int scl) {
	pinMode(default_sda_pin = sda, RESERVED);
	pinMode(default_scl_pin = scl, RESERVED);
    flush();
}

size_t TwoWire::requestFrom(uint8_t address, size_t size, bool sendStop) {
    if (size > BUFFER_LENGTH)
        size = BUFFER_LENGTH;
    rxBufferIndex = 0;
    return rxBufferLength = twi_readFrom(address, rxBuffer, size, sendStop) == 0 ? size : 0;
}

void TwoWire::beginTransmission(uint8_t address) {
    transmitting = 1;
    txAddress = address;
    txBufferIndex = 0;
}

uint8_t TwoWire::endTransmission(uint8_t sendStop) {
    int8_t ret = twi_writeTo(txAddress, txBuffer, txBufferIndex, sendStop);
    txBufferIndex = 0;
    transmitting = 0;
    return ret;
}

size_t TwoWire::write(uint8_t data) {
    if (!transmitting)
		throw HTException("Illegal I2C Write", "Attempted to write without beginning transmission");
    if (txBufferIndex >= BUFFER_LENGTH)
		throw HTException("Illegal I2C Write", "Outgoing message buffer exceeded max length of %d.", BUFFER_LENGTH);
	txBuffer[txBufferIndex++] = data;
    return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity) {
	for (size_t i = 0; i < quantity; ++i)
		write(data[i]);
    return quantity;
}

int TwoWire::available(void) { return rxBufferLength != rxBufferIndex; }
int TwoWire::read(void) { return rxBufferIndex < rxBufferLength ? rxBuffer[rxBufferIndex++] : -1; }
int TwoWire::peek(void) { return rxBufferIndex < rxBufferLength ? rxBuffer[rxBufferIndex] : -1; }

void TwoWire::flush(void) {
    rxBufferIndex = 0;
    rxBufferLength = 0;
    txBufferIndex = 0;
}

TwoWire Wire;