#include "LTC6811_2.h"
#include <SPI.h>
#include <string.h>
#include <stdio.h>

// SPI slave select pin, as required for Teensy 3.2
#define SS 10
// SPI alternate pin definitions (not needed unless using Teensy alternate SPI pins)
#define MOSI 7 // pin 11 by default
#define MISO 8 // pin 12 by default
#define SCLK 14 // pin 13 by default
// SPI mode; see LTC6811 datasheet page 44
#define SPI_MODE 3
// SPI speed in Hz. LTC6811 max data transfer rate is 1 Mbps, which requires a speed of 1 MHz
#define SPI_SPEED 1000000
// SPI bit ordering. LTC6811 requires big endian ordering
#define SPI_BIT_ORDER MSBFIRST


// SPI write
void spi_write(uint8_t *cmd, uint8_t *cmd_pec, uint8_t *data, uint8_t *data_pec, int data_size) {
    SPI.beginTransaction(SPISettings(SPI_SPEED,SPI_BIT_ORDER, SPI_MODE));
    digitalWrite(SS, low);
    SPI.transfer(cmd[0]);
    SPI.transfer(cmd[1]);
    SPI.transfer(cmd_pec[0]);
    SPI.transfer(cmd_pec[1]);
    for (int i = 0; i < data_size; i++) {
        SPI.transfer(data[i]);
    }
    SPI.transfer(data_pec[0]);
    SPI.transfer(data_pec[1]);
    digitalWrite(SS, high);
    SPI.endTransaction();
}
// SPI read


// returns the address of the specific LTC6811-2 chip to send command to
uint8_t LTC6811_2::get_cmd_address() {
    return 0x80 | (address << 3);
}

/* Packet Error Code (PEC) generated using algorithm specified on datasheet
 * PEC array structure:
 * pec[0] = bits 14 downto 7 of PEC
 * pec[1] = bits 6 downto 0 of PEC; LSB of pec[1] is a padded zero as per datasheet
 */
void LTC6811_2::generate_pec(uint8_t *value, uint8_t *pec, int num_bytes) {
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
/* Write Configuration Register Group A */
void LTC6811_2::wrcfga(Reg_Group_Config reg_group) {
    /* cmd array structure as related to datasheet:
     * cmd[0] = CMD0;
     * cmd[1] = CMD1;
     */
    uint8_t cmd[2] = {get_cmd_address(), 0x1};
    uint8_t cmd_pec[2];
    uint8_t data[5];
    uint8_t data_pec[2];
    generate_pec(&cmd, &cmd_pec, 2);
    buffer = reg_group.buf();
    for (int i = 0; i < 5; i++) {
        data[i] = buffer[i];
    }
    generate_pec(&data, &data_pec, 5);
    spi_write(&cmd, &cmd_pec, &data, &data_pec, 5);
}

void LTC6811_2:wrsctrl(Reg_Group_S_Ctrl reg_group) {
    
};



int main() {

	Reg_Group_Aux_A a(0xA324,0x243A,0xBBBB);

	uint8_t* buffer = a.buf();

	for(int i = 0; i < 6; i++) {
		printf("0x%02x ", buffer[i]);
	}

	return 0;
}
