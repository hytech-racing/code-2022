#include "../../../Libraries/HyTech17_Library/HyTech17.h"
#include "../../../Libraries/XBTools/XBTools.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <iomanip>

#define MESSAGE_LENGTH 12

int main(int argc, char **argv) {
    char *serial_port;
    if (argc >= 2) {
        serial_port = argv[1];
    } else {
        std::cout << "Must provide serial port as argument" << std::endl;
        exit(1);
    }

    // open serial port as low-level file descriptor
    std::cout << "Using serial port " << serial_port << std::endl;
    int xbee_device = open(serial_port, O_NONBLOCK | O_NOCTTY);
    if (xbee_device != -1) {
        // holds raw data
        uint8_t read_buf[MESSAGE_LENGTH];
        // CAN id
        uint8_t id = 0;
        // CAN message
        uint8_t message[8];
        std::cout << "Serial port initialized" << std::endl;
        unsigned char c;
        int result = read(xbee_device, &c, 1);
        std::cout << "got first character: ";
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) c << std::endl;
        int index = 0;
        while (result != -1) {
            if (c == 0x00) {
                if (index > 0) {
                    // unstuff COBS
                    uint8_t cobs_buf[MESSAGE_LENGTH];
                    int decoded = cobs_decode(read_buf, MESSAGE_LENGTH, cobs_buf);
                    std::cout << std::hex << "COBS-decoded data: " << cobs_buf << std::endl;
                    if (decoded) {
                        // COBS decoded some data, now check the checksum
                        int checksum = read_buf[MESSAGE_LENGTH - 1] << 8 | read_buf[MESSAGE_LENGTH - 2];
                        uint8_t raw_msg[MESSAGE_LENGTH - 2];
                        memcpy(raw_msg, read_buf, MESSAGE_LENGTH - 2);
                        int calc_checksum = fletcher16(raw_msg, MESSAGE_LENGTH - 2);
                        std::cout << std::hex << "Msg Checksum: " << checksum << " -- Calc Checksum: " << calc_checksum << std::endl;
                        if (calc_checksum == checksum) {
                            // do stuff with data
                            id = cobs_buf[0];
                            int length = cobs_buf[1];
                            memcpy(message, cobs_buf + 2, length);
                        }
                    }
                    index = 0;
                }
            } else {
                if (index < 12) {
                    read_buf[index] = c;
                    index++;
                } else {
                    // we missed a \0
                    index = 0;
                }
            }
            if (id != 0) {
                std::cout << std::hex << "Received ID: " << id << " -- " << message << std::endl;
            }
            result = read(xbee_device, &c, 1);
            std::cout << "got first character: ";
            std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) c << std::endl;
        }
        std::cout << "errno: " << strerror(errno) << std::endl;
        close(xbee_device);
    }
}