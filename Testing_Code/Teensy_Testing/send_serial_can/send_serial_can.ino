#include <FlexCAN.h>
#include "HyTech17.h"

#define XBEE_PKT_LEN 15

FlexCAN CAN(500000);
static CAN_message_t msg;
BMS_detailed_temperatures bms_detailed_temperatures(1, 11, 22, 33);
MC_command_message command_msg(160, 0, 1, 1, 0, 200);
int msg_id = 0;

// Original data:     DA00081B01602100268C
// COBS-encoded data: 2DA11481B21622113268C0

void setup() {
    CAN.begin();
    Serial.begin(115200);
    //Serial.println("Initialized serial");
}
void loop() {
    delay(1000);
     if (msg_id % 3 == 1) {
      msg.id = ID_MC_FAULT_CODES;
      msg.len = sizeof(CAN_message_mc_fault_codes_t);
      memset(msg.buf, 0, 8);
      msg.buf[5] = 8;
    } else if (msg_id % 2 == 0) {
      msg.id = ID_BMS_DETAILED_TEMPERATURES;
      msg.len = sizeof(CAN_message_bms_detailed_temperatures_t);
      bms_detailed_temperatures.write(msg.buf);
    } else if (msg_id % 2 == 1) {
      msg.id = ID_MC_COMMAND_MESSAGE;
      msg.len = sizeof(CAN_message_mc_command_message_t);
      command_msg.write(msg.buf);
    }
    serial_send_message(msg);
    msg_id++;
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
    // delim (1) + checksum (2) + id (4) + length (1) + length
    uint8_t xb_buf[XBEE_PKT_LEN] = {0};
    memcpy(xb_buf, &msg.id, sizeof(msg.id));        // msg id
    memcpy(xb_buf + sizeof(msg.id), &msg.len, sizeof(msg.len));     // msg len
    memcpy(xb_buf + sizeof(msg.id) + sizeof(msg.len), msg.buf, msg.len); // msg contents

    // calculate checksum
    uint16_t checksum = fletcher16(xb_buf, XBEE_PKT_LEN - 2);
    //Serial.print("CHECKSUM: ");
    //Serial.println(checksum, HEX);
    memcpy(&xb_buf[XBEE_PKT_LEN - 2], &checksum, sizeof(uint16_t));

//    for (int i = 0; i < XBEE_PKT_LEN; i++) {
//      Serial.print(xb_buf[i], HEX);
//      Serial.print(" ");
//    }
//    Serial.println();

    uint8_t cobs_buf[2 + XBEE_PKT_LEN];
    cobs_encode(xb_buf, XBEE_PKT_LEN+1, cobs_buf);
    cobs_buf[XBEE_PKT_LEN+1] = 0x0;

//    for (int i = 0; i < XBEE_PKT_LEN+2; i++) {
//      Serial.print(cobs_buf[i], HEX);
//      Serial.print(" ");
//    }
//    Serial.println();

    int written = Serial.write(cobs_buf, 2 + XBEE_PKT_LEN);
//    Serial.print("Wrote ");
//    Serial.print(written);
//    Serial.println(" bytes");
}
