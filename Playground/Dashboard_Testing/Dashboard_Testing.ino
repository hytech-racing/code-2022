// This code sets up one pin on the I/O Expander as an input and reads it, and one pin as an output that outputs the reading from the first pin.

// pins set for each number
//0 : 0 1 2 3 4 5 
//1 : 1 2 
//2 : 0 1 3 4 6 
//3 : 0 1 2 3 6
//4 : 1 2 5 6 
//5 : 0 2 3 5 6 
//6 : 0 2 3 4 5 6
//7 : 0 1 2
//8 : 0 1 2 3 4 5 6 
//9 : 0 1 2 5 6


#include <SPI.h>
#include <MCP23S08.h>

const uint8_t ADDR = 0; // address of the chip when  all address pins are grounded
const uint8_t CS = 8;  // pin on Teensy that is used as Chip Select
const uint8_t number_pins[10] = 
                     {0b00000010, 0b10011110, 0b00100100, 0b10011110,
                      0b10011000, 0b01001000,  0b01000000, 0b00011110,
                      0b00000001, 0b00011000};

MCP23S08 expander(ADDR, CS);  // MCP23S17 object for the chip with the specified address and chip select pin

void setup() {
  Serial.begin(9600);
  delay(1000);
  expander.begin(); 
  // Assign all pins as outputs and write all high to start
  for (int i = 0; i < 8; i++) {
    expander.pinMode(i, OUTPUT);
    expander.digitalWrite(i, HIGH);
  }
}

void loop() {
  //expander.digitalWrite(0, value);     // output the reading from pin 7 on pin 0
}
