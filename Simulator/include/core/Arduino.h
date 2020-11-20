#pragma once

#define ARDUINO 101

#include <cstring>

#include "BoardDef.h"
#include "Interrupts.h"
#include "Serial.h"
#include "Simulator.h"

#define LOW 0
#define HIGH 1

typedef uint8_t byte;

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
inline void delay (unsigned long long time) 		{ Simulator::sys_time += time; }