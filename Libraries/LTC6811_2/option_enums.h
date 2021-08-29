#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

//datasheet defined choices
enum class ADC_MODE {
	MODE_ZERO=0,
	FAST,
	NORMAL,
	FILTERED
};

enum class DISCHARGE {
	DISCHARGE_NOT_PERMITTED = 0,
	DISCHARGE_PERMITTED
};

enum class CELL_SELECT {
	ALL_CELLS = 0,
	CELL_1_7,
	CELL_2_8,
	CELL_3_9,
	CELL_4_10,
	CELL_5_11,
	CELL_6_12
};

enum class OPEN_WIRE_CURRENT_PUP {
	PULL_DOWN=0,
	PULL_UP
};

enum class SELF_TEST_MODE{
	SELF_TEST_1=1,
	SELF_TEST_2
};

enum class GPIO_SELECT {
	ALL_GPIO_AND_REF_2=0,
	GPIO_1,
	GPIO_2,
	GPIO_3,
	GPIO_4,
	GPIO_5,
	REF_2
};

enum class STATUS_GROUP_SELECT {
	ALL_GROUPS=0,
	SC,
	ITMP,
	VA,
	VD
};

enum class S_CONTROL {
	S_DRIVE_HIGH = 0,
	S_1_PULSE,
	S_2_PULSES,
	S_3_PULSES,
	S_4_PULSES,
	S_5_PULSES,
	S_6_PULSES,
	S_7_PULSES,
	S_DRIVE_LOW
};