#include <FlexCAN.h>
#include <cobs.h>
#include "HyTech17.h"

FlexCAN CAN(500000);
static CAN_message_t msg;
void setup() {
    CAN.begin();
    Serial.begin(115200);
}
void loop() {
    delay(1000);
    BMS_detailed_temperatures bms_detailed_temperatures(12, millis(), millis() / 2, millis() / 3);
    msg.id += 1;
    msg.len = sizeof(CAN_message_bms_detailed_temperatures_t);
    bms_detailed_temperatures.write(msg.buf);
    serial_send_message(msg);
}

// copied from Wikipedia lol
uint16_t fletcher16(const uint8_t *data, size_t len) {
    uint32_t c0, c1;
    unsigned int i;

    for (c0 = c1 = 0; len >= 5802; len -= 5802) {
        for (i = 0; i < 5802; ++i) {
            c0 = c0 + *data++;
            c1 = c1 + c0;
        }
        c0 = c0 % 255;
        c1 = c1 % 255;
    }
    for (i = 0; i < len; ++i) {
        c0 = c0 + *data++;
        c1 = c1 + c0;
    }
    c0 = c0 % 255;
    c1 = c1 % 255;
    return (c1 << 8 | c0);
}
void serial_send_message(CAN_message_t msg) {
    // checksum (2) + id (id-size) + length (1) + length (length)
    uint8_t message_size = sizeof(uint16_t) + sizeof(msg.id) + sizeof(uint8_t) + msg.len;
    uint8_t string[message_size];
    memcpy(string, &msg.id, sizeof(msg.id));
    memcpy(string + sizeof(msg.id), &msg.len, sizeof(uint8_t));
    memcpy(string + sizeof(msg.id) + sizeof(uint8_t), msg.buf, msg.len);
    uint16_t fletcher = fletcher16(string, sizeof(msg.id) + sizeof(uint8_t) + msg.len);
    memcpy(string + sizeof(msg.id) + sizeof(uint8_t) + msg.len, &fletcher, sizeof(uint16_t));
    Serial.print("Fletcher Checksum: ");
    Serial.println(fletcher);
    Serial.print("Data: ");
    for (int i = 0; i < message_size; i++) {
        Serial.print((uint8_t)string[i], HEX);
    }

    uint8_t cobs_string[message_size];
    cobs_encode(string, message_size, cobs_string);
    Serial.print("COBS-encoded data: ");
    for (int i = 0; i < message_size; i++) {
        Serial.print((uint8_t)string[i], HEX);
    }
    Serial.print("\n");
}