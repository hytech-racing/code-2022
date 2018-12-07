/**
 * This program will generate HyTech-compatible Xbee messages from the data specified.
 * The construction of the Xbee message is identical to that used by the Rear ECU.
 * Final Xbee message will be output to the system console.
 * -Ryan Gallaway, 10/21/2018
 */
#include "../../Libraries/HyTech_CAN/HyTech_CAN.h"
#include "../../Libraries/XBTools/XBTools.h"
#include <iostream>
#include <fstream>
#include <iomanip>

#define XBEE_PKT_LEN 15

typedef struct {
    uint32_t id;
    uint8_t ext;
    uint8_t len;
    uint16_t timeout;
    uint8_t buf[8];
} CAN_message_t;

static CAN_message_t xb_msg;

/**
 * Writes data currently in global xb_msg variable to the Xbee serial bus.
 * Calculates Fletcher checksum and byte-stuffs so that messages are
 * delimited by 0x0 bytes.
 * 
 * returns: number of bytes written to the Xbee serial bus
 */
int write_xbee_data() {
    /*
     * DECODED FRAME STRUCTURE:
     * [ msg id (4) | msg len (1) | msg contents (8) | checksum (2) ]
     * ENCODED FRAME STRUCTURE:
     * [ fletcher (1) | msg id (4) | msg len (1) | msg contents (8) | checksum (2) | delimiter (1) ]
     */
    uint8_t xb_buf[XBEE_PKT_LEN];
    memcpy(xb_buf, &xb_msg.id, sizeof(xb_msg.id));        // msg id
    memcpy(xb_buf + sizeof(xb_msg.id), &xb_msg.len, sizeof(uint8_t));     // msg len
    memcpy(xb_buf + sizeof(xb_msg.id) + sizeof(uint8_t), xb_msg.buf, xb_msg.len); // msg contents

    // calculate checksum
    uint16_t checksum = fletcher16(xb_buf, XBEE_PKT_LEN - 2);
    std::cout << "CHECKSUM: ";
    std::cout << std::hex << checksum << std::endl;
    memcpy(&xb_buf[XBEE_PKT_LEN - 2], &checksum, sizeof(uint16_t));

    uint8_t cobs_buf[2 + XBEE_PKT_LEN];
    cobs_encode(xb_buf, XBEE_PKT_LEN, cobs_buf);
    cobs_buf[XBEE_PKT_LEN+1] = 0x0;

    std::ofstream outfile;
    outfile.open("message.txt", std::ios::out);
    for (int i = 0; i < XBEE_PKT_LEN+2; i++) {
        std::cout << std::hex << std::showbase << std::setfill('0') << std::setw(2) << (int)cobs_buf[i];
        if (i < XBEE_PKT_LEN + 1) {
            std::cout << ",";
        }
        outfile << cobs_buf[i];
    }
    std::cout << std::endl;
    outfile.close();

    //memset(xb_buf, 0, sizeof(CAN_message_t));
    return 1;
}

int main() {
    //MC_command_message command = MC_command_message(250, 0, 0, 1, 0, 0);
    FCU_status command = FCU_status(3, 0, 1);
    xb_msg.id = ID_FCU_STATUS;
    //xb_msg.len = sizeof(CAN_message_mc_command_message_t);
    xb_msg.len = sizeof(CAN_message_fcu_status_t);
    write_xbee_data();
    return 0;
}

