

//TODO: change to enum casses instead?
//not sure if it would really be any more beneficial

namespace LTC6811_2 {

//datasheet defined choices
enum ADC_MODE {
	MODE_ZERO=0,
	FAST,
	NORMAL,
	FILTERED
};

enum DISCHARGE {
	DISCHARGE_NOT_PERMITTED = 0,
	DISCHARGE_PERMITTED
};

enum CELL_SELECT {
	ALL_CELLS = 0,
	CELL_1_7,
	CELL_2_8,
	CELL_3_9,
	CELL_4_10,
	CELL_5_11,
	CELL_6_12
};

enum OPEN_WIRE_CURRENT_PUP {
	PULL_DOWN=0,
	PULL_UP
};

enum SELF_TEST_MODE{
	SELF_TEST_1=1,
	SELF_TEST_2
};

enum GPIO_SELECT {
	ALL_GPIO_AND_REF_2=0,
	GPIO_1,
	GPIO_2,
	GPIO_3,
	GPIO_4,
	GPIO_5,
	REF_2
};

enum STATUS_GROUP_SELECT {
	ALL_GROUPS=0,
	SC,
	ITMP,
	VA,
	VD
};


//better name later
enum CELL_DISCHARGE {
	CELL_1 = 1<<0,
	CELL_2 = 1<<1,
	CELL_3 = 1<<2,
	CELL_4 = 1<<3,
	CELL_5 = 1<<4,
	CELL_6 = 1<<5,
	CELL_7 = 1<<6,
	CELL_8 = 1<<7,
	CELL_9 = 1<<8,
	CELL_10 = 1<<9,
	CELL_11 = 1<<10,
	CELL_12 = 1<<11
};

enum GPIO_CONFIG {
	GPIO1_SET = 1<<0,
	GPIO2_SET = 1<<1,
	GPIO3_SET = 1<<2,
	GPIO4_SET = 1<<3,
	GPIO5_SET = 1<<4,
};

enum S_CONTROL {
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
};