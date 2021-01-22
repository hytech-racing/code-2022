#pragma once

#include <cstring>
#include <cmath>

#include "BoardDef.h"
#include "Interrupts.h"
#include "Serial.h"
#include "Simulator.h"

#define LOW 0
#define HIGH 1

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

typedef uint8_t byte;
typedef bool boolean;

// defined in microcontroller code
extern void setup();
extern void loop();

// used to manage pin i/o
inline bool digitalRead(int pin) 					{ return Simulator::io[pin].vehicle_read(); }
inline unsigned analogRead(int pin)					{ return Simulator::io[pin].vehicle_read(); }
inline void digitalWrite(int pin, bool value) 		{ Simulator::io[pin].vehicle_write(value); }
inline void analogWrite(int pin, unsigned value)	{ Simulator::io[pin].vehicle_write(value); }
inline void pinMode(int pin, unsigned mode) 		{ Simulator::io[pin].vehicle_pinMode(mode); }

// used for time management
inline unsigned long long millis()					{ return Simulator::sys_time; }
inline unsigned long long micros()					{ return millis() * 1000 + Simulator::sys_us; }
inline void delay (unsigned long long time) 		{ Simulator::sys_time += time; }
inline void delayMicroseconds (unsigned long long us) {
	Simulator::sys_us += us % 1000;
	Simulator::sys_time += us / 1000 + Simulator::sys_us / 1000;
	Simulator::sys_us %= 1000;
}

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}