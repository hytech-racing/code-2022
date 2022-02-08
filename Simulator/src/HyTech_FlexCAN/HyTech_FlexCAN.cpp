#include "HyTech_FlexCAN.h"

#include "MockCAN.h"
#include "Interrupts.h"

inline void setBase(uint32_t& packed, int tx, int rx) { packed = tx << 16 | rx; }

inline void getBase(uint32_t packed, int& tx, int& rx) {
	tx = packed >> 16;
	rx = packed & 0xFFFF;
}

FlexCAN::FlexCAN(uint32_t baud, uint8_t id, uint8_t txAlt, uint8_t rxAlt) {
	defaultMask = { 0, 0, 0 };

	if (baud != 500000)
		throw CANException("CAN bus baud rate must be 500000");

	#ifdef HYTECH_ARDUINO_TEENSY_32
		int txPin = 3, rxPin = 4;
	#endif

	#ifdef HYTECH_ARDUINO_TEENSY_35
		int txPin = 3, rxPin = 4;
	#endif

	setBase(flexcanBase, txPin, rxPin);
}

void FlexCAN::begin(const CAN_filter_t &mask) {
	int txPin, rxPin;
	getBase(flexcanBase, txPin, rxPin);
	pinMode(txPin, RESERVED);
	pinMode(rxPin, RESERVED);

	#ifdef HYTECH_ARDUINO_TEENSY_32
		if (Interrupts::enabled() && FLEXCAN0_IMASK1 != FLEXCAN_IMASK1_BUF5M)
			throw CANException("If interrupts enabled, Teensy 3.2 requires FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M");
	#endif
}

void FlexCAN::setFilter(const CAN_filter_t &filter, uint8_t n) {
	static CAN_filter_t allMasks[8];
	if (n == -1)
		for (int i = 0; i < 8; ++i)
			allMasks[i] = filter;
	else if (n < 8)
		allMasks[n] = filter;

	bool valid = true;
	for (int i = 0; valid && i < 8; ++i)
		if (allMasks[i].rtr != filter.rtr || allMasks[i].ext != filter.ext || allMasks[i].id != filter.id)
			valid = false;
	if (valid) defaultMask = filter;
	else defaultMask.id = ~0u;
}
void FlexCAN::end(void) {
	int txPin, rxPin;
	getBase(flexcanBase, txPin, rxPin);
	pinMode(txPin, UNUSED); pinMode(rxPin, UNUSED);

	#ifdef HYTECH_ARDUINO_TEENSY_32
		FLEXCAN0_IMASK1 = 0;
	#elif defined (HYTECH_ARDUINO_TEENSY_35)
		FLEXCAN0_MCR = 0xFFFFFFFF;
	#endif
}

int FlexCAN::available(void) { return true; }

int FlexCAN::write(const CAN_message_t &msg) {
	if (defaultMask.id == ~0u)
		throw CANException("CAN configuration not valid");
	MockCAN::vehicle_write(msg);

	#ifdef HYTECH_ARDUINO_TEENSY_35
	    if (FLEXCAN0_MCR == 0xFFFDFFFF)
			MockCAN::vehicle_write(msg);
	#endif

	return true;
}

int FlexCAN::read(CAN_message_t &msg) {
	if (defaultMask.id == ~0u)
		throw CANException("CAN configuration not valid");
	do {
		if (!MockCAN::vehicle_read(msg))
			return false;
	} while (msg.rtr == defaultMask.rtr && msg.id == defaultMask.id && msg.ext == defaultMask.ext);
	return true;
}

#ifdef HYTECH_ARDUINO_TEENSY_32
	unsigned long long FLEXCAN0_IMASK1 = 0;
#endif
#ifdef HYTECH_ARDUINO_TEENSY_35
	unsigned long long FLEXCAN0_MCR = 0xFFFFFFFF;
#endif
