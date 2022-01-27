// This code sets up one pin on the I/O Expander as an input and reads it, and one pin as an output that outputs the reading from the first pin.

#include <SPI.h>
#include <MCP23S08.h>

const uint8_t ADDR = 0; // address of the chip when  all address pins are grounded
const uint8_t CS = 8;  // pin on Teensy that is used as Chip Select

MCP23S08 expander(ADDR, CS);  // MCP23S17 object for the chip with the specified address and chip select pin

void setup() {
  Serial.begin(9600);
  delay(1000);
  expander.begin(); 

  expander.pinMode(0, OUTPUT);  // set pin 0 as an output (corresponds to GPIOA0)
}

void loop() {
  int value = expander.digitalRead(7); // read pin 7 and store its value
  Serial.println(value);               // print the reading from pin 7
  expander.digitalWrite(0, value);     // output the reading from pin 7 on pin 0
}
