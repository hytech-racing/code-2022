#include "LTC6811_2.h"

#include <string.h>
#include <stdio.h>


int main() {

	Reg_Group_Aux_A a(0xA324,0x243A,0xBBBB);

	uint8_t* buffer = a.buf();

	for(int i = 0; i < 6; i++) {
		printf("0x%02x ", buffer[i]);
	}

	return 0;
}