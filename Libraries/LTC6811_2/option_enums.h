#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

//datasheet defined choices
enum class ADC_MODE : uint8_t {
	MODE_ZERO = 0x0,
	FAST = 0x1,
	NORMAL = 0x2,
	FILTERED = 0x3
};

enum class DISCHARGE : uint8_t {
	DISCHARGE_NOT_PERMITTED = 0x0,
	DISCHARGE_PERMITTED = 0x1
};

enum class CELL_SELECT : uint8_t {
	ALL_CELLS = 0x0,
	CELL_1_7 = 0x1,
	CELL_2_8 = 0x2,
	CELL_3_9 = 0x3,
	CELL_4_10 = 0x4,
	CELL_5_11 = 0x5,
	CELL_6_12 = 0x6
};

enum class OPEN_WIRE_CURRENT_PUP {
	PULL_DOWN=0,
	PULL_UP
};

enum class SELF_TEST_MODE{
	SELF_TEST_1=1,
	SELF_TEST_2
};

enum class GPIO_SELECT : uint8_t {
	ALL_GPIO_AND_REF_2=0,
	GPIO_1 = 0x1,
	GPIO_2 = 0x2,
	GPIO_3 = 0x3,
	GPIO_4 = 0x4,
	GPIO_5 = 0x5,
	REF_2 = 0x6
};

enum class STATUS_GROUP_SELECT {
	ALL_GROUPS=0,
	SC,
	ITMP,
	VA,
	VD
};