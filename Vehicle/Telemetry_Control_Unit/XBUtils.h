#pragma once

#include <XBTools.h>

#define XB Serial2
#define XBEE_PKT_LEN 15

#define send_xbee(can_id, can_msg, xb_msg) { \
	can_msg.write(xb_msg.buf); \
	xb_msg.len = sizeof(can_msg); \
	xb_msg.id = can_id; \
	HT::XBUtil::write(xb_msg); \
}

namespace HT {
namespace XBUtil {

	// namespace {
	// 	void printBuffer(const uint8_t* const buf, const size_t size) {
	// 		for (size_t i = 0; i < size; ++i) {
	// 			if (buf[i] < 0x10)
	// 				Serial.print('0');
	// 			Serial.print(buf[i], HEX);
	// 			Serial.print(" ");
	// 		}
	// 	}
	// }

	int write(CAN_message_t& xb_msg) {
		// [ msg id (4) | msg len (1) | msg contents (8) | checksum (2) ]
		// [ fletcher (1) | msg id (4) | msg len (1) | msg contents (8) | checksum (2) | delimiter (1) ]

		uint8_t xb_buf [XBEE_PKT_LEN];
		memset(xb_buf, 0, XBEE_PKT_LEN);
		memcpy(xb_buf, &xb_msg.id, sizeof(xb_msg.id));
		memcpy(xb_buf + sizeof(xb_msg.id), &xb_msg.len, sizeof(uint8_t));
		memcpy(xb_buf + sizeof(xb_msg.id) + sizeof(uint8_t), xb_msg.buf, xb_msg.len);

		uint16_t checksum = fletcher16(xb_buf, XBEE_PKT_LEN - 2);
		memcpy(&xb_buf[XBEE_PKT_LEN - 2], &checksum, sizeof(uint16_t));

		uint8_t cobs_buf [XBEE_PKT_LEN + 2];
		cobs_encode(xb_buf, XBEE_PKT_LEN, cobs_buf);
		cobs_buf[XBEE_PKT_LEN + 1] = 0x0;

		// Serial.print("CHECKSUM: "); Serial.println(checksum, HEX);
		// Serial.print("DECODED:  "); printBuffer(xb_buf, XBEE_PKT_LEN);
		// Serial.print("ENCODED:  "); printBuffer(cobs_buf, XBEE_PKT_LEN + 2);

		int written = XB.write(cobs_buf, 2 + XBEE_PKT_LEN);
		// Serial.print("Wrote "); Serial.print(written); Serial.println(" bytes");

		return written;
	}
}
}