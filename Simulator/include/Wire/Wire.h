#pragma once

#include <cstddef>
#include <stdint.h>
#include <unordered_map>

#define BUFFER_LENGTH 128

class TwoWire {
private:
    static uint8_t rxBuffer[];
    static uint8_t rxBufferIndex;
    static uint8_t rxBufferLength;

    static uint8_t txAddress;
    static uint8_t txBuffer[];
    static uint8_t txBufferIndex;
    static uint8_t txBufferLength;

    static uint8_t transmitting;

public:
    TwoWire();
    void begin(int sda, int scl);
    void begin();

    void beginTransmission(uint8_t);
    uint8_t endTransmission(uint8_t sendStop = true);
    size_t requestFrom(uint8_t address, size_t size, bool sendStop = true);

    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *, size_t);

    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
    virtual void flush(void);
};

extern TwoWire Wire;