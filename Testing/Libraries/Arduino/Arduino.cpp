#include "Arduino.h"

bool digitalRead(int pin) { return io[pin].sim_value(); };
void digitalWrite(int pin, bool value) { io[pin].sim_write(value); }
unsigned analogRead(int pin) { return io[pin].sim_value(); }
void analogWrite(int pin, unsigned value) { io[pin].sim_write(value); }
void pinMode(int pin, unsigned mode) { io[pin].sim_pinMode(mode); }

unsigned long long millis() { return Simulator::sys_time; }
void delay (unsigned long long time) { Simulator::sys_time += time; }