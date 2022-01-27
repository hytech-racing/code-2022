/*
	MCP23S08.h
	Microchip MCP23S08 SPI I/O Expander Class for Teensy
	Leonid Pozdneev
	December 2018

	This library is a reworked version of the library posted on the Arduino website. It implements the same features
	and all function names are the same. The implementation of some functions is also similar (and comments too).

	Features Implemented (by word and by bit):
		I/O Direction
		Pull-ups On/Off
		Input inversion
		Output write
		Input read
		Generic byte read
		Generic byte write

	Interrupt features are not implemented in this version (will be added in the future).

	NOTE:	Addresses below are only valid when IOCON.BANK=0 (register addressing mode)
         	This means one of the control register values can change register addresses! (refer to the datasheet)
         	The default values is 0, so that's how we're using it. (Functionality when IOCON.BANK = 1 will be added in the future.)

         	All registers except ICON (0xA and 0xB) are paired as A/B for each 8-bit GPIO port.
        	Comments identify the port's name, and notes on how it is used.

        	*THIS CLASS ENABLES THE ADDRESS PINS ON ALL CHIPS ON THE BUS WHEN THE FIRST CHIP OBJECT IS INSTANTIATED!


  	USAGE:	All Read/Write functions except wordWrite are implemented in two different ways.
        	Individual pin values are set by referencing "pin #" and On/Off, Input/Output or High/Low where
        	portA represents pins 0-7 and portB 8-15. So to set the most significant bit of portB, set pin # 15.
        	To Read/Write the values for the entire chip at once, a word mode is supported buy passing a
        	single argument to the function as 0x(portB)(portA). I/O mode Output is represented by 0.
        	The wordWrite function was to be used internally, but was made public for advanced users to have
        	direct and more efficient control by writing a value to a specific register pair.
*/

#include <Arduino.h>


#ifndef MCP23S08_h
#define MCP23S08_h

/*===========================================
	REGISTERS
---------------------------------------------*/
#define IODIR     (0x00)  // I/O Direction Registers
// 1 = Input (default), 0 = Output
#define IPOL      (0x01)  // Polarity Registers
// 1 = Inverted (low reads as 1), 0 = Normal (default)(low reads as 0)
#define GPINTEN   (0x02)  // Interrupt-On-Change Pins
// 1 = interrupt-on-change enabled, 0 = interrupt-on-change disabled (default)


#define DEFVAL    (0x03)  // Default Value Registers
// Opposite of what is here will trigger an interrupt (default = 0)

#define INTCON    (0x04)  // Interrupt-On-Change Control Registers
// 1 = pin is compared to DEFVAL, 0 = pin is compared to previous value (default)

#define IOCON      (0x05)  // I/O Expander Configuration Register
// Also Configiration Register

#define GPPU      (0x06)  // GPIO Pull-Up Resistor Registers
// 1 = pull-up enabled, 0 = pull-up disabled (default)

#define INTF      (0x07)  // Interrupt Flag Registers
// READ ONLY: 1 = This Pin Triggered the Interrupt

#define INTCAP    (0x8)  // Interrupt Captured Value For Port Registers
// READ ONLY: State of the Pin at the Time the Interrupt Occurred

#define GPIO      (0x9)  // General Purpose I/O Port Registers
// Value on the Port - Writing Sets Bits in the Output Latch

#define OLAT      (0xA)  // Output Latch Registers
// 1 = Latch High, 0 = Latch Low (default) Reading Returns Latch State, Not Port Value!


class MCP23S08 {
  public:
    MCP23S08(uint8_t, uint8_t);              // Constructor to instantiate a discrete IC as an object, address 0-7, chipSelect any valid pin
    MCP23S08(uint8_t, uint8_t, unsigned int);
    void begin();                            // Start the SPI Bus and write default values to all registers
    //void wordWrite(uint8_t, uint8_t);        // Typically only used internally, but allows the user to write any register pair if needed, so it's public
    void byteWrite(uint8_t, uint8_t);        // Typically only used internally, but allows the user to write any register if needed, so it's public
    void pinMode(uint8_t, uint8_t);          // Sets the mode (input or output) of a single I/O pin
    void pinMode(uint8_t);                   // Sets the mode (input or output) of all I/O pins at once
    void pullupMode(uint8_t, uint8_t);       // Selects internal 100k input pull-up of a single I/O pin
    void pullupMode(uint8_t);                // Selects internal 100k input pull-up of all I/O pins at once
    void inputInvert(uint8_t, uint8_t);      // Selects input state inversion of a single I/O pin (writing 1 turns on inversion)
    void inputInvert(uint8_t);               // Selects input state inversion of all I/O pins at once (writing a 1 turns on inversion)
    void digitalWrite(uint8_t, uint8_t);     // Sets an individual output pin HIGH or LOW
    void digitalWrite(uint8_t);              // Sets all output pins at once. If some pins are configured as input, those bits will be ignored on write
    uint8_t digitalRead(uint8_t);            // Reads an individual input pin
    uint8_t byteRead(uint8_t);               // Reads an individual register and returns the byte. Argument is the register address
    unsigned int digitalRead(void);          // Reads all input  pins at once. Be sure it ignore the value of pins configured as output!
  private:
    void init(uint8_t, uint8_t, unsigned int);
    uint8_t _address;                        // Address of the MCP23S08 in use
	uint8_t _cs;                             // chip select pin
    uint8_t _modeCache;                 // Caches the mode (input/output) configuration of I/O pins
    uint8_t _pullupCache;               // Caches the internal pull-up configuration of input pins (values persist across mode changes)
    uint8_t _invertCache;               // Caches the input pin inversion selection (values persist across mode changes)
    uint8_t _outputCache;               // Caches the output pin state of pins
    uint8_t _SPIspeed;
 };

#endif //MCP23S08