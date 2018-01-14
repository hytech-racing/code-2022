#include <FlexCAN.h>
#include "HyTech17.h"

FlexCAN CAN(500000);
static CAN_message_t msg;
void setup() {
    CAN.begin();
    Serial.begin(115200);
    Serial.println("Initialized serial");
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

/* Stuffs "length" bytes of data at the location pointed to by
 * "input", writing the output to the location pointed to by
 * "output". Returns the number of bytes written to "output".
 *
 * Remove the "restrict" qualifiers if compiling with a
 * pre-C99 C dialect.
 */
size_t cobs_encode(const uint8_t * input, size_t length, uint8_t * out)
{
    size_t read_index = 0;
    size_t write_index = 1;
    size_t code_index = 0;
    uint8_t code = 1;

    while(read_index < length)
    {
        if(input[read_index] == 0)
        {
            out[code_index] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        }
        else
        {
            out[write_index++] = input[read_index++];
            code++;
            if(code == 0xFF)
            {
                out[code_index] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }

    out[code_index] = code;

    return write_index;
}

/* Unstuffs "length" bytes of data at the location pointed to by
 * "input", writing the output * to the location pointed to by
 * "output". Returns the number of bytes written to "output" if
 * "input" was successfully unstuffed, and 0 if there was an
 * error unstuffing "input".
 *
 * Remove the "restrict" qualifiers if compiling with a
 * pre-C99 C dialect.
 */
size_t cobs_decode(const uint8_t * input, size_t length, uint8_t * out)
{
    size_t read_index = 0;
    size_t write_index = 0;
    uint8_t code;
    uint8_t i;

    while(read_index < length)
    {
        code = input[read_index];

        if(read_index + code > length && code != 1)
        {
            return 0;
        }

        read_index++;

        for(i = 1; i < code; i++)
        {
            out[write_index++] = input[read_index++];
        }
        if(code != 0xFF && read_index != length)
        {
            out[write_index++] = '\0';
        }
    }

    return write_index;
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
    Serial.println(fletcher, HEX); // remember it's little endian
    Serial.print("Original Data:     ");
    for (int i = 0; i < message_size; i++) {
        Serial.print((uint8_t)string[i], HEX);
    }

    uint8_t cobs_string[message_size];
    size_t cobs_length = cobs_encode(string, message_size, cobs_string);
    Serial.print("\nCOBS-encoded data: ");
    for (int i = 0; i < cobs_length; i++) {
        Serial.print((uint8_t)cobs_string[i], HEX);
    }
    uint8_t decoded[message_size];
    cobs_decode(cobs_string, cobs_length, decoded);
    Serial.print("\nDecoded COBS data: ");
    for (int i = 0; i < message_size; i++) {
        Serial.print((uint8_t)decoded[i], HEX);
    }
    Serial.println("\n");
}
