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
//See this link for LED segment information https://www.geeksforgeeks.org/seven-segment-displays/
//These encodings correspond to the pins on the IO expander (0 = LSB to 7 = MSB)
//Only pins 0 - 6 are needed (as the decimal on teh 7-segment is not used)
//e.g. To encode 0 on the 7 segment, segments A-F must light up, while G is left off
//The 7-seg LEDs share a common anode, meaning the pins on the IO expander must be set low
//for the segment to light up. The encoding of 0 starts from the LSB, adding 6 zeros to illuminate A-F
//a 1 is then added to keep G off and the 7th pin is left off. This created the encoding 0b01000000 which
//is sent via SPY to the IO expander

//REWRITE THE ABOVE
const uint8_t number_pins[10] = 
                     {0b01000000, 0b01111001, 0b00100100, 0b00110000,
                      0b00011001, 0b00010010, 0b00000010, 0b01111000,
                      0b10000000, 0b00011000};

MCP23S08 expander(ADDR, CS);  // MCP23S17 object for the chip with the specified address and chip select pin

void setup() {
  
  delay(1000);
//  //Begin SPI communication with IO Expander
  expander.begin(); 
//  // Assign all pins on expander as outputs and write all high to start
  for (int i = 0; i < 8; i++) {
    expander.pinMode(i, OUTPUT);
    expander.digitalWrite(i, HIGH);
  }
  pinMode(A1, INPUT);//INERTIA
  pinMode(1, OUTPUT);//IMD LED
  pinMode(A6,INPUT);//SSOK
  pinMode(A7, INPUT);//SHUTDOWNH
}

void loop() {
//  //cycle through number encodings to display each on the 7-segment using the expander
//  for (int i = 0; i < 10; i++) {
//    expander.digitalWrite(number_pins[i]);
//    delay(1000);
//  }


  
  int testIMDLED = 0;
  int testAMSLED = 0;

  bool codeList[10] = {};
  int displayList[10] = {};

  int INERTIA = digitalRead(A1);
  int SSOK = digitalRead(A6);
  int SHUTDOWN_H = digitalRead(A7);
  
  if(SSOK && !(INERTIA))
  {
    codeList[1] = true;
  }
  if (INERTIA && !(SHUTDOWN_H))
  {
    codeList[2] = true ;
  }
  if(testIMDLED)
  {
    codeList[3] = true;
  }
  if(testAMSLED)
  {
    codeList[4] = true;
  }

// DEBUG CODE TO DISPLAY 1 and 3, REMOVE WHEN ACTUAL SIGNALS EXIST
codeList[1] = true;
codeList[3] = true;


//convert codes to displays
  int count = 0;
  for (int i = 0; i < 10; i++)
  {
    if(codeList[i])
    {
      displayList[count] = i;
    }
    count++;
  }

//debug

//display codes
  for (int i = 0; i < 10; i++)
  {
    if(displayList[i] != 0)
    {
      expander.digitalWrite(number_pins[displayList[i]]);
      delay(1000);
    }
    else
    {
      
    }
  }
  //Test Inertia
  /*
  int val = digitalRead(A1);
  if (val == HIGH) {
    digitalWrite(1, HIGH);
  }
  */

  delay(100);
}
