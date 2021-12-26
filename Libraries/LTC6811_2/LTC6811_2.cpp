#include "LTC6811_2.h"
#include <SPI.h>
#include <string.h>
#include <stdio.h>

// SPI slave select pin, as required for Teensy 3.2
#define SS 10
// SPI alternate pin definitions (not needed unless using Teensy 3.2 alternate SPI pins)
#define MOSI 7 // pin 11 by default
#define MISO 8 // pin 12 by default
#define SCLK 14 // pin 13 by default
// SPI mode; see LTC6811 datasheet page 44
#define SPI_MODE SPI_MODE3
// SPI speed in Hz. LTC6811 max data transfer rate is 1 Mbps, which requires a speed of 1 MHz
#define SPI_SPEED 1000000
// SPI bit ordering. LTC6811 requires big endian ordering
#define SPI_BIT_ORDER MSBFIRST

/* Note; SPI transfer is a simultaneous send/receive; the spi_write function disregards the received data, and
 * the spi_read function sends dummy values in order to read in values from the slave device. */
// SPI write
void LTC6811_2::spi_write(uint8_t *cmd, uint8_t *cmd_pec, uint8_t *data, uint8_t *data_pec) {
    SPI.beginTransaction(SPISettings(SPI_SPEED,SPI_BIT_ORDER, SPI_MODE));
    digitalWrite(SS, low);
    SPI.transfer(cmd[0]);
    SPI.transfer(cmd[1]);
    SPI.transfer(cmd_pec[0]);
    SPI.transfer(cmd_pec[1]);
    for (int i = 0; i < 6; i++) {
        SPI.transfer(data[i]);
    }
    SPI.transfer(data_pec[0]);
    SPI.transfer(data_pec[1]);
    digitalWrite(SS, high);
    SPI.endTransaction();
}
// SPI read; IF CODE DOES NOT WORK, THIS IS A GOOD PLACE TO START DEBUGGING
void LTC6811_2::spi_read(uint8_t *cmd, uint8_t* cmd_pec, uint8_t *data_in) {
    SPI.beginTransaction(SPISettings(SPI_SPEED,SPI_BIT_ORDER, SPI_MODE));
    digitalWrite(SS, low);
    SPI.transfer(cmd[0]);
    SPI.transfer(cmd[1]);
    SPI.transfer(cmd_pec[0]);
    SPI.transfer(cmd_pec[1]);
    // read in data and PEC; bytes 0 to 5 data bytes; bytes 6 to 7 PEC bytes
    for (int i = 0; i < 8; i++) {
        data_in[i] = SPI.transfer(0); // transfer dummy value over SPI in order to read bytes into data
    }
    digitalWrite(SS, high);
    SPI.endTransaction();
}


// returns the address of the specific LTC6811-2 chip to send command to
uint8_t LTC6811_2::get_cmd_address() {
    return 0x80 | (this->address << 3);
}

/* Packet Error Code (PEC) generated using algorithm specified on datasheet page 53-54
 * PEC array structure:
 * pec[0] = bits 14 downto 7 of PEC
 * pec[1] = bits 6 downto 0 of PEC; LSB of pec[1] is a padded zero as per datasheet
 */
void LTC6811_2::generate_pec(const uint8_t *value, uint8_t *pec, int num_bytes) {
    pec[0] = 0b00000000;
    pec[1] = 0b00100000;
    uint8_t din, in0, in3, in4, in7, in8, in10, in14;
    for (int i = 0; i < num_bytes; i++) {
        for (int j = 7; j >= 0; j--) {
            //isolate current din bit
            din = value[i] >> j & 0x1;
            //generate in bits for next PEC
            in0 = (din ^ (pec[0] >> 7 & 0x1)) << 1;
            in3 = (in0 >> 1 ^ (pec[1] >> 3 & 0x1)) << 4;
            in4 = (in0 >> 1 ^ (pec[1] >> 4 & 0x1)) << 5;
            in7 = (in0 >> 1 ^ (pec[1] >> 7 & 0x1));
            in8 = (in0 >> 1 ^ (pec[0] & 0x1)) << 1;
            in10 = (in0 >> 1 ^ (pec[0] >> 2 & 0x1)) << 3;
            in14 = (in0 >> 1 ^ (pec[0] >> 6 & 0x1)) << 7;
            //generate new PEC bit strings for next iteration
            pec[0] = in14 | in10 | in8 | in7 | (pec[0] << 1 & 0b01110100);
            pec[1] = in4 | in3 | in0 | (pec[1] << 1 & 0b11001100);
        }
    }
}

// Write register commands
// General write register group handler
void LTC6811_2::write_register_group(uint16_t cmd_code, const uint8_t *buffer) {
    /* cmd array structure as related to datasheet:
     * cmd[0] = CMD0;
     * cmd[1] = CMD1;
     */
    auto *cmd_code_bytes = reinterpret_cast<uint8_t *>(cmd_code);
    uint8_t cmd[2] = {static_cast<uint8_t>(get_cmd_address() | cmd_code_bytes[0]), cmd_code_bytes[1]};
    uint8_t cmd_pec[2];
    uint8_t data[6];
    uint8_t data_pec[2];
    // generate PEC from command bytes
    generate_pec(cmd, cmd_pec, 2);
    for (int i = 0; i < 6; i++) {
        data[i] = buffer[i];
    }
    // generate PEC from data bytes
    generate_pec(data, data_pec, 6);
    // write out via SPI
    spi_write(cmd, cmd_pec, data, data_pec);
}
// Write Configuration Register Group A
void LTC6811_2::wrcfga(Reg_Group_Config reg_group) {
    write_register_group(0x1, reg_group.buf());
}
// Write S Control Register Group
void LTC6811_2::wrsctrl(Reg_Group_S_Ctrl reg_group) {
    write_register_group(0x14, reg_group.buf());
}
// Write PWM Register group
void LTC6811_2::wrpwm(Reg_Group_PWM reg_group) {
    write_register_group(0x20, reg_group.buf());
}
// Write COMM Register Group
void LTC6811_2::wrcomm(Reg_Group_COMM reg_group) {
    write_register_group(0x721, reg_group.buf());
}


// Read register commands
// general read register group handler
void LTC6811_2::read_register_group(uint16_t cmd_code, uint8_t *data) {
    // bytes 0 to 5 data bytes; bytes 6 to 7 PEC bytes
    uint8_t data_in[8];
    auto *cmd_code_bytes = reinterpret_cast<uint8_t *>(cmd_code);
    uint8_t cmd[2] = {static_cast<uint8_t>(get_cmd_address() | cmd_code_bytes[0]), cmd_code_bytes[1]};
    uint8_t cmd_pec[2];
    uint8_t data_pec[2];
    // Generate PEC from command bytes
    generate_pec(cmd, cmd_pec, 2);
    // read in via SPI
    spi_read(cmd, cmd_pec, data_in);
    // generate PEC from read-in data bytes
    generate_pec(data_in, data_pec, 6);
    // Check if the PEC locally generated on the data that is read in matches the PEC that is read in
    if (data_pec[0] == data_in[6] || data_pec[1] == data_in[7]) {
        throw 1; // should throw actual exception derived from std::exception but I'm tired and don't wanna learn how
    }
    // After confirming matching PECs, add the data that was read in to the array that was passed into the function
    for (int i = 0; i < 6; i++) {
        data[i] = data_in[i];
    }
}
// Read Configuration Register Group A
Reg_Group_Config LTC6811_2::rdcfga() {
    uint8_t buffer[6];
    // If PECs do not match, then construct default object
    try {
        read_register_group(0x2, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read Cell Voltage Register Group A
Reg_Group_Cell_A LTC6811_2::rdcva() {
    uint8_t buffer[6];
    try {
        read_register_group(0x4, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read Cell Voltage Register Group B
Reg_Group_Cell_B LTC6811_2::rdcvb() {
    uint8_t buffer[6];
    try {
        read_register_group(0x6, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read Cell Voltage Register Group C
Reg_Group_Cell_C LTC6811_2::rdcvc() {
    uint8_t buffer[6];
    try {
        read_register_group(0x8, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read Cell Voltage Register Group D
Reg_Group_Cell_D LTC6811_2::rdcvd() {
    uint8_t buffer[6];
    try {
        read_register_group(0xA, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read Auxiliary Register Group A
Reg_Group_Aux_A LTC6811_2::rdauxa() {
    uint8_t buffer[6];
    try {
        read_register_group(0xC, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read Status Register Group A
Reg_Group_Status_A LTC6811_2::rdstata() {
    uint8_t buffer[6];
    try {
        read_register_group(0x10, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read Status Register Group B
Reg_Group_Status_B LTC6811_2::rdstatb() {
    uint8_t buffer[6];
    try {
        read_register_group(0x12, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read S Control Register Group
Reg_Group_S_Ctrl LTC6811_2::rdsctrl() {
    uint8_t buffer[6];
    try {
        read_register_group(0x16, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read PWM Register Group
Reg_Group_PWM LTC6811_2::rdpwm() {
    uint8_t buffer[6];
    try {
        read_register_group(0x22, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}
// Read COMM Register Group
Reg_Group_COMM LTC6811_2::rdcomm() {
    uint8_t buffer[6];
    try {
        read_register_group(0x722, buffer);
        return {buffer};
    } catch (int error) {
        return {};
    }
}