#include "../../../Libraries/HyTech_CAN/HyTech_CAN.h"
#include "../../../Libraries/XBTools/XBTools.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <termios.h>

#define MESSAGE_LENGTH 15

int configure_port(int fd)      // configure the port
{
	struct termios port_settings;      // structure to store the port settings in

	cfsetispeed(&port_settings, B115200);    // set baud rates
	cfsetospeed(&port_settings, B115200);

	port_settings.c_cflag &= ~PARENB;    // set no parity, stop bits, data bits
	port_settings.c_cflag &= ~CSTOPB;
	port_settings.c_cflag &= ~CSIZE;
	port_settings.c_cflag |= CS8;
    port_settings.c_cflag |= CRTSCTS;
	
	tcsetattr(fd, TCSANOW, &port_settings);    // apply the settings to the port
	return(fd);

} //configure_port

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
    int xbee_device = configure_port(open(serial_port, O_NOCTTY));
    if (xbee_device != -1) {
        // holds raw data
        uint8_t read_buf[MESSAGE_LENGTH+2];
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
                    // std::cout << std::endl;
                    // unstuff COBS
                    uint8_t cobs_buf[MESSAGE_LENGTH];
                    int decoded = cobs_decode(read_buf, MESSAGE_LENGTH + 2, cobs_buf);
                    // std::cout << "\nCOBS-decoded data: ";
                    // for (int i = 0; i < 15; i++) {
                    //     std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) cobs_buf[i] << " ";
                    // }
                    // std::cout << "Decoded " << decoded << " bytes" << std::endl;
                    if (decoded) {
                        // COBS decoded some data, now check the checksum
                        int checksum = cobs_buf[MESSAGE_LENGTH - 1] << 8 | cobs_buf[MESSAGE_LENGTH - 2];
                        uint8_t raw_msg[MESSAGE_LENGTH - 2];
                        memcpy(raw_msg, cobs_buf, MESSAGE_LENGTH - 2);
                        // std::cout << "Non-checksum data: ";
                        // for (uint8_t num : raw_msg) {
                        //     std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) num << " ";
                        // }
                        int calc_checksum = fletcher16(raw_msg, MESSAGE_LENGTH - 2);
                        // std::cout << std::hex << "Msg Checksum: " << checksum << " -- Calc Checksum: " << calc_checksum << std::endl;
                        if (calc_checksum == checksum) {
                            // do stuff with data
                            memcpy(&id, &raw_msg, 4);
                            int length = raw_msg[4];
                            memcpy(message, raw_msg + 5, 8);
                            // std::cout << std::hex << "Received ID: " << id << " -- ";
                            // for (int i = 0; i < length; i++) {
                            //     std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) message[i] << " ";
                            // }
                            // std::cout << std::endl;

                            if (id == ID_RCU_STATUS) {
                                RCU_status rcu_status(message);
                                std::cout << "Battery voltage: " << std::dec << rcu_status.get_glv_battery_voltage() / 10 << std::endl;
                                std::cout << "Temperature: " << rcu_status.get_temperature() / 100 << std::endl;
                                std::cout << std::endl;
                            }
                            if (id == ID_BMS_VOLTAGES) {
                                BMS_voltages bms_voltages(message);
                                std::cout << "Cell avg voltage: " << std::dec << bms_voltages.get_average() / 10 << std::endl;
                                std::cout << "Cell low voltage: " << bms_voltages.get_low() / 10 << std::endl;
                                std::cout << "Cell high voltage: " << bms_voltages.get_high() / 10 << std::endl;
                                std::cout << "Total pack voltage: " << bms_voltages.get_total() / 10 << std::endl;
                                std::cout << std::endl;
                            }
                            if (id == ID_FCU_STATUS) {
                                FCU_status fcu_status(message);
                                std::cout << "FCU State: " << std::dec << fcu_status.get_state() << std::endl;
                                if (fcu_status.get_accelerator_implausibility()) {
                                    std::cout << "Accelerator Implausibility Detected" << std::endl;
                                }
                                if (fcu_status.get_accelerator_boost_mode()) {
                                    std::cout << "Accel Boost Mode ACTIVE" << std::endl;
                                }
                                if (fcu_status.get_brake_implausibility()) {
                                    std::cout << "Brake Implausibility Detected" << std::endl;
                                }
                                if (fcu_status.get_brake_pedal_active()) {
                                    std::cout << "Brake pedal active" << std::endl;
                                }
                                std::cout << std::endl;
                            }
                        }
                    }
                    index = 0;
                }
            } else {
                if (index < MESSAGE_LENGTH+2) {
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