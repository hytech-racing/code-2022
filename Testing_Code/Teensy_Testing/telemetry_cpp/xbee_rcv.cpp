#include "../../../Libraries/HyTech17_Library/HyTech17.h"
#include "../../../Libraries/XBTools/XBTools.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <iomanip>

#define MESSAGE_LENGTH 16

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
    int xbee_device = open(serial_port, O_NOCTTY);
    if (xbee_device != -1) {
        // holds raw data
        uint8_t read_buf[MESSAGE_LENGTH];
        // CAN id
        int id = 0;
        // CAN message
        uint8_t message[8];
        int length;
        std::cout << "Serial port initialized" << std::endl;
        unsigned char c;
        int result = read(xbee_device, &c, 1);
        int index = 0;
        while (result != -1) {
            if (c == 0x00) {
                if (index > 0) {
                    // std::cout << "Raw data: ";
                    // for (uint8_t num : read_buf) {
                    //     std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) num << " ";
                    // }
                    // unstuff COBS
                    uint8_t cobs_buf[15];
                    int decoded = cobs_decode(read_buf, 16, cobs_buf);
                    // std::cout << "\nCOBS-decoded data: ";
                    // for (int i = 0; i < 15; i++) {
                    //     std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) cobs_buf[i] << " ";
                    // }
                    // std::cout << "Decoded " << decoded << " bytes" << std::endl;
                    if (decoded) {
                        // COBS decoded some data, now check the checksum
                        int checksum = cobs_buf[MESSAGE_LENGTH - 2] << 8 | cobs_buf[MESSAGE_LENGTH - 3];
                        uint8_t raw_msg[MESSAGE_LENGTH - 3];
                        memcpy(raw_msg, cobs_buf, MESSAGE_LENGTH - 3);
                        // std::cout << "Non-checksum data: ";
                        // for (uint8_t num : raw_msg) {
                        //     std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) num << " ";
                        // }
                        int calc_checksum = fletcher16(raw_msg, MESSAGE_LENGTH - 3);
                        // std::cout << std::hex << "Msg Checksum: " << checksum << " -- Calc Checksum: " << calc_checksum << std::endl;
                        if (calc_checksum == checksum) {
                            // do stuff with data
                            memcpy(&id, &raw_msg[0], 4);
                            int length = raw_msg[4];
                            memcpy(message, raw_msg + 5, length);
                            std::cout << std::hex << "Received ID: " << id << " -- ";
                            for (uint8_t num : message) {
                                std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) num << " ";
                            }
                            std::cout << std::endl;
                        }
                    }
                    index = 0;
                }
            } else {
                if (index < 16) {
                    read_buf[index] = c;
                    index++;
                } else {
                    // we missed a \0
                    index = 0;
                }
            }
            result = read(xbee_device, &c, 1);
        }
        std::cout << "errno: " << strerror(errno) << std::endl;
        close(xbee_device);
    }
}