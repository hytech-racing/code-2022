#include "LTC6811_2.h"

#include <string.h>
#include <stdio.h>

uint8_t LTC6811_2::get_cmd_address() {
    return 0x80 | (address << 3);
}

/* Packet Error Code (PEC) generated following datasheet algorithm
 * PEC array structure:
 * pec[0] = bits 14 downto 7 of PEC
 * pec[1] = bits 6 downto 0 of PEC; LSB of pec[1] is a padded zero as per datasheet
 *
 */
void LTC6811_2::generate_pec(uint8_t *byte, uint8_t *pec, int num_bytes) {
    pec[0] = 0b00000000;
    pec[1] = 0b00100000;
    uint8_t din, in0, in3, in4, in7, in8, in10, in14;
    for (int i = 0; i < num_bytes; i++) {
        for (int j = 7; j >= 0; j--) {
            //isolate current din bit
            din = byte[i] >> j & 0x1;
            //generate in bits for next PEC
            in0 = (din ^ (pec[0] >> 7 & 0x1)) << 1;
            in3 = (in0 >> 1 ^ (pec[1] >> 3 & 0x1)) << 4;
            in4 = (in0 >> 1 ^ (pec[1] >> 4 & 0x1)) << 5;
            in7 = (in0 >> 1 ^ (pec[1] >> 7 & 0x1));
            in8 = (in0 >> 1 ^ (pec[0] & 0x1)) << 1;
            in10 = (in0 >> 1 ^ (pec[0] >> 2 & 0x1)) << 3;
            in14 = (in0 >> 1 ^ (pec[0] >> 6 & 0x1)) << 7;
            //generate new PEC bitstrings for next iteration
            pec[0] = in14 | in10 | in8 | in7 | (pec[0] << 1 & 0b01110100);
            pec[1] = in4 | in3 | in0 | (pec[1] << 1 & 0b11001100);
        }
    }
}

void LTC6811_2::wrcfga(Reg_Group_Config reg_group) {
    /* cmd array structure as related to datasheet:
     * cmd[0] = CMD0;
     * cmd[1] = CMD1;
     */
    uint8_t cmd[2] = {get_cmd_address(), 0x1};
    uint8_t pec[2];
    uint_8t cmd_pec[2] = generate_pec()
    reg_group.buf();

}

int main() {

	Reg_Group_Aux_A a(0xA324,0x243A,0xBBBB);

	uint8_t* buffer = a.buf();

	for(int i = 0; i < 6; i++) {
		printf("0x%02x ", buffer[i]);
	}

	return 0;
}