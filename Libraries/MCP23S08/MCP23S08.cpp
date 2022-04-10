/*
    MCP23S08.h
    Microchip MCP23S08 SPI I/O Expander Class for Teensy
    Youssef Jaafar
    based on MCP23S217 library by Leonid Pozdneev
    January 2022

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

    NOTE:   Addresses below are only valid when IOCON.BANK=0 (register addressing mode)
            This means one of the control register values can change register addresses! (refer to the datasheet)
            The default values is 0, so that's how we're using it. (Functionality when IOCON.BANK = 1 will be added in the future.)

            All registers except ICON (0xA and 0xB) are paired as A/B for each 8-bit GPIO port.
            Comments identify the port's name, and notes on how it is used.

            *THIS CLASS ENABLES THE ADDRESS PINS ON ALL CHIPS ON THE BUS WHEN THE FIRST CHIP OBJECT IS INSTANTIATED!


    USAGE:  All Read/Write functions except wordWrite are implemented in two different ways.
            Individual pin values are set by referencing "pin #" and On/Off, Input/Output or High/Low. So to set the most significant bit, set pin # 7.

            To Read/Write the values for the entire chip at once, a word mode is supported buy passing a
            single argument to the function as 0x(portB)(portA). I/O mode Output is represented by 0.
            The wordWrite function was to be used internally, but was made public for advanced users to have
            direct and more efficient control by writing a value to a specific register pair.
*/

#include <SPI.h>       // Arduino IDE SPI library - uses AVR hardware SPI features
#include "MCP23S08.h"  // Header files for this class

// Defines to keep logical information symbolic go here

#ifndef HIGH
#define    HIGH          (1)
#endif

#ifndef LOW
#define    LOW           (0)
#endif

#ifndef ON
#define    ON            (1)
#endif

#ifndef OFF
#define    OFF           (0)
#endif

//

#define OPCODE_WRITE (0b01000000)
#define OPCODE_READ  (0b01000001)

// Constructor to instantiate an instance of MCP23S08 to a specific chip (address)

MCP23S08::MCP23S08(uint8_t address, uint8_t cs) {
    init(address, cs, SPI_CLOCK_DIV8);
};

MCP23S08::MCP23S08(uint8_t address, uint8_t cs, unsigned int SPIspeed) {
    init(address, cs, SPIspeed);
};

void MCP23S08::init(uint8_t address, uint8_t cs, unsigned int SPIspeed) {
    _address     = address;
    _cs          = cs;
    _SPIspeed    = SPIspeed;

    _modeCache   = 0xFF;              // Default I/O mode is all input, 0xFF
    _pullupCache = 0x00;              // Default output state is all off, 0x00
    _outputCache = 0x00;              // Default pull-up state is all off, 0x00
    _invertCache = 0x00;              // Default input inversion state is not inverted, 0x00

}

// Start the SPI Bus, write default values to all registers, and enable the hardware address pins (IOCON.HAEN = 1)

void MCP23S08::begin() {
    ::pinMode(_cs, OUTPUT);               // Set Chip Select pin as an output
    ::digitalWrite(_cs, HIGH);            // Set Chip Select HIGH (chip de-selected)

    SPI.begin();                          // Start the SPI bus

    byteWrite(IODIR, 0xFF);              // Write the default value (0xFF) in IODIR register

    for (int i = 1; i < 11; i++) {        // This loop writes default values (0x00) to the rest of the registers
        byteWrite(i, 0x00);
        ::delay(10);
    }

    byteWrite(IOCON, 0b00001000);         // Enable the hardware address pins
}

// Write a byte to a register
// Arguments: register address, the value to write

void MCP23S08::byteWrite(uint8_t reg, uint8_t value) {
    SPI.beginTransaction(SPISettings (_SPIspeed, MSBFIRST, SPI_MODE0));       // Begin the SPI transaction (default settings from the SPI library)
    ::digitalWrite(_cs, LOW);                                                 // Bring the chip select pin LOW
    SPI.transfer(OPCODE_WRITE | _address << 1);                               // Send the MCP23S08 opcode, chip address, and the write bit
    SPI.transfer(reg);                                                        // Send the register addreess
    SPI.transfer(value);                                                      // Send the value to write
    ::digitalWrite(_cs, HIGH);                                                // Bring the chip select pin HIGH
    SPI.endTransaction();                                                     // End the SPI transaction
}

// // Write a word to a register pair, LSB to first register, MSB to next higher value register
// // Arguments: address of the first register,  and the word

// void MCP23S08::wordWrite(uint8_t reg, unsigned int word) {
//     byteWrite(reg, (uint8_t) word);             // Write the LSB to the fisrt register
//     byteWrite(reg + 1, (uint8_t) (word >> 8));  // Write the MSB to the register with the next higher value register
// }

// Configure the mode of a pin (input or output)
// Arguments: pin number, mode (1 = Input, 0 = Output)

void MCP23S08::pinMode(uint8_t pin, uint8_t mode) {
    if (pin < 0 || pin > 7) return;  // Check if the pin value is between 0 and 15; the function does nothing if the pin value is outside the range

    if (mode == OUTPUT)               // Determine the mode
        _modeCache &= ~(1 << pin);    // If mode is OUTPUT, write 0 in the appropriate place of the mode cache
    else
        _modeCache |= (1 << pin);     // If mode value is anything other than 0, write 1 in the appropriate place of the mode cache

    byteWrite(IODIR,  _modeCache);         // Write the mode cache to IODIR
}

// Configure the mode of all 8 pins
// Arguments: mode (1 byte; LSB = pin 0; 1 = input, 0 = output)

void MCP23S08::pinMode(uint8_t mode) {
    _modeCache = mode;                               // Update the mode cache
    byteWrite(IODIR, _modeCache);         // Write the the mode cache to IODIR
}

// Configure the internal pull-up of a pin
// Arguments: pin number (0-15; 0 = pin 0 of the register A, 15 = pin 7 of the register B), mode (1 = Enable pull-up, 0 = Disable pull-up)

void MCP23S08::pullupMode(uint8_t pin, uint8_t mode) {
    if (pin < 0 || pin > 7) return;

    if (mode == OFF)
        _pullupCache &= ~(1 << pin);
    else
        _pullupCache |= (1 << pin);

    byteWrite(GPPU,  _pullupCache);
}

// Configure the internal pull-ups of all pins
// Arguments: mode (16 bit word; the lowest bit of the word is pin 0 of the register A, the highest bit of the word is pin 7 of the register B; 1 = Enable, 0 = Disable)

void MCP23S08::pullupMode(uint8_t mode) {
    _pullupCache = mode;
    byteWrite(GPPU, (uint8_t) _pullupCache);
}

// Configure the inversion of a pin
// Arguments: pin (0-15; 0 = pin 0 of the register A, 15 = pin 7 of the register B), mode (1 = Enable inversion, 0 = Disable inversion)

void MCP23S08::inputInvert(uint8_t pin, uint8_t mode) {
    if (pin < 0 || pin > 7) return;

    if (mode == OFF)
        _invertCache &= ~(1 << pin);
    else
        _invertCache |= (1 << pin);

    byteWrite(IPOL, _invertCache);
}

// Connfigure the inversion of all pins
// Arguments: mode (16 bit word; the lowest bit of the word is pin 0 of the register A, the highest bit of the word is pin 7 of the register B; 1 = Enable, 0 = Disable)

void MCP23S08::inputInvert(uint8_t mode) {
    _invertCache = mode;
    byteWrite(IPOL, _invertCache);
}

// Write to a pin
// Arguments: pin (0-15; 0 = pin 0 of the register A, 15 = pin 7 of the register B), value

void MCP23S08::digitalWrite(uint8_t pin, uint8_t value) {
    if (pin < 0 || pin > 7) return;

    if (value == LOW)
        _outputCache &= ~(1 << pin);
    else
        _outputCache |= (1 << pin);

    byteWrite(OLAT, _outputCache);
}

// Write to all pins (pins that are not set as outputs are ignored)
// Arguments: value (16 bit word; the lowest bit of the word is pin 0 of the register A, the highest bit of the word is pin 7 of the register B; 1 = Enable, 0 = Disable)

void MCP23S08::digitalWrite(uint8_t value) {
    _outputCache = value;
    byteWrite(OLAT, (uint8_t) _outputCache);
}

// Read all pins (pins that are not set as inputs are ignored)
// Arguments: none
// Return: readings from both GPIO registers (16 bit word; the LSB is GPIOA, the MSB is GPIOB)

unsigned int MCP23S08::digitalRead(void) {
    unsigned int reading;               // Initialize a variable to hold the read values to be returned

    reading = byteRead(GPIO);          // Read the register A pins and store as the LSB of the reading variable

    return reading;                     // Return the reading
}

// Read a register
// Arguments: register address
// Return: the value in the specified register

uint8_t MCP23S08::byteRead(uint8_t reg) {
    uint8_t reading;                                                          // Initialize a variable to hold the read values to be returned

    SPI.beginTransaction(SPISettings (_SPIspeed, MSBFIRST, SPI_MODE0));  // Begin the SPI transaction (default settings from the SPI library)
    ::digitalWrite(_cs, LOW);                                                 // Bring the chip select pin LOW
    SPI.transfer(OPCODE_READ | _address << 1);                                // Send the MCP23S08 opcode, chip address, and the read bit
    SPI.transfer(reg);                                                        // Send the register addreess
    reading =  SPI.transfer(0);                                               // Read the register (0 is dummy data to send)
    ::digitalWrite(_cs, HIGH);                                                // Bring the chip select pin HIGH
    SPI.endTransaction();                                                     // End the SPI transaction

    return reading;                                                           // Return the reading
}

// Read a pin (if the pin is not set as an input, returns 0)
// Arguments: pin (0-15; 0 = pin 0 of the register A, 15 = pin 7 of the register B)
// Return: the value of the pin

uint8_t MCP23S08::digitalRead(uint8_t pin) {
    if (pin < 0 || pin > 7) return 0;               // Check if the pin value is between 0 and 15; the function returns 0 if the pin value is outside the range

    return digitalRead() & (1 << pin) ? HIGH : LOW;
}