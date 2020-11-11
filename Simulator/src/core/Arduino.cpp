#include "Arduino.h"

bool digitalRead(int pin) { return io[pin].vehicle_read(); };
void digitalWrite(int pin, bool value) { io[pin].vehicle_write(value); }
unsigned analogRead(int pin) { return io[pin].vehicle_read(); }
void analogWrite(int pin, unsigned value) { io[pin].vehicle_write(value); }
void pinMode(int pin, unsigned mode) { io[pin].vehicle_pinMode(mode); }

unsigned long long millis() { return Simulator::sys_time; }
void delay (unsigned long long time) { Simulator::sys_time += time; }