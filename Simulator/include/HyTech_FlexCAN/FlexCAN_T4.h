#pragma once
#include "BoardDef.h"

#ifdef HYTECH_ARDUINO_TEENSY_40

#include <MockCAN.h>
#include <stdint.h>

enum CAN_PINSET_T4 {
	CAN1 = (23 << 8) | 22,
	CAN2 = (0 << 8) | 1
};

typedef struct CAN_message_t {
  uint32_t id = 0;          // can identifier
  uint16_t timestamp = 0;   // FlexCAN time when message arrived
  uint8_t idhit = 0; // filter that id came from
  struct {
    bool extended = 0; // identifier is extended (29-bit)
    bool remote = 0;  // remote transmission request packet type
    bool overrun = 0; // message overrun
    bool reserved = 0;
  } flags;
  uint8_t len = 8;      // length of data
  uint8_t buf[8] = { 0 };       // data
  uint8_t mb = 0;       // used to identify mailbox reception
  uint8_t bus = 0;      // used to identify where the message came from when events() is used.
  bool seq = 0;         // sequential frames
} CAN_message_t;

#define assert(cond, msg) if (!(cond)) throw CANException(msg);

template <CAN_PINSET_T4 pins>
class FlexCAN_T4 {
public:
	inline void begin() {
		assert(status == IDLE, "Already called FlexCAN_T4.begin()");
		status = SETUP;
		pinMode(pins >> 8, RESERVED);
		pinMode(pins & 0xFF, RESERVED);
	}

	inline void setBaudRate(uint32_t) { 
		assert(status != IDLE, "Must begin() FlexCAN_T4 before setting baud rate.");
		assert(status != RUNNING, "Double set baud rate for FlexCAN_T4");
		started = RUNNING;
	}

	inline void end() {
		status = IDLE;
		enableInterrupts = false;
		interrupts.clear();
		pinMode(pins >> 8, UNUSED);
		pinMode(pins & 0xFF, UNUSED);
	}

	inline void onReceive(void (cb*)(CAN_message_t&)) {
		assert(enableInterrupts, "Must enable MB interrupts before adding onReceive callback");
		interrupts.push_back(cb);
	}

	inline void events() {
		assert(enableInterrupts, "Must enable MB interrupts before calling events()");
		CAN_message_t& msg;
		while (read(msg))
			for (auto& foo : interrupts)
				foo(msg);
	}

	inline void enableMBInterrupts() 		{ assert(status != IDLE, "FlexCAN_T4 not running (enableMBInterrupts)"); enableInterrupts = true; }
	inline int read(CAN_message_t& msg) 	{ assert(status == RUNNING, "FlexCAN_T4 not running (read)");	return MockCAN::vehicle_read(msg); }
	inline int write(CAN_message_t&) 		{ assert(status == RUNNING, "FlexCAN_T4 not running (write)");	return MockCAN::vehicle_write(msg); }
private:
	enum class Status { IDLE, SETUP, RUNNING; }
	Status status;
	bool enableInterrupts;
	std::list<void(*)(CAN_message_t&)> interrupts;
};

#undef assert
#endif