#include "xbee_rcv_thread.h"
#include <QtCore>

#define MESSAGE_LENGTH 15

xbee_rcv_thread::xbee_rcv_thread() {
    moveToThread(this);
}

xbee_rcv_thread::~xbee_rcv_thread() {
    close(this->xbee_device);
}

void xbee_rcv_thread::run() {
    exec();
}

void xbee_rcv_thread::exec() {
    if (this->xbee_device != -1) {
        uint8_t read_buf[MESSAGE_LENGTH+2];
        // CAN id
        int id = 0;
        // CAN message
        uint8_t message[8];
        unsigned char c;
        int result = read(xbee_device, &c, 1);
        int index = 0;
        while (result != -1) {
            if (c == 0x00) {
                if (index > 0) {
                    // unstuff COBS
                    uint8_t cobs_buf[MESSAGE_LENGTH];
                    int decoded = cobs_decode(read_buf, MESSAGE_LENGTH + 2, cobs_buf);

                    if (decoded) {
                        // COBS decoded some data, now check the checksum
                        int checksum = cobs_buf[MESSAGE_LENGTH - 1] << 8 | cobs_buf[MESSAGE_LENGTH - 2];
                        uint8_t raw_msg[MESSAGE_LENGTH - 2];
                        memcpy(raw_msg, cobs_buf, MESSAGE_LENGTH - 2);
                        int calc_checksum = fletcher16(raw_msg, MESSAGE_LENGTH - 2);

                        if (calc_checksum == checksum) {
                            // do stuff with data
                            memcpy(&id, &raw_msg, 4);
                            int length = raw_msg[4];
                            memcpy(message, raw_msg + 5, 8);

                            emit updated(id, length, QByteArray(reinterpret_cast<char*>(message), 8));
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
        emit xbee_error(strerror(errno));
    }
    emit xbee_error(strerror(errno));
}

int xbee_rcv_thread::configure_port(QString port) {
    QByteArray portba = port.toLatin1();
    int fd = open(portba.data(), O_NOCTTY);
    struct termios port_settings;      // structure to store the port settings in

    cfsetispeed(&port_settings, B115200);    // set baud rates
    cfsetospeed(&port_settings, B115200);

    port_settings.c_cflag &= ~PARENB;    // set no parity, stop bits, data bits
    port_settings.c_cflag &= ~CSTOPB;
    port_settings.c_cflag &= ~CSIZE;
    port_settings.c_cflag |= CS8;
    port_settings.c_cflag |= CRTSCTS;

    tcsetattr(fd, TCSANOW, &port_settings);    // apply the settings to the port

    std::cout << "Serial port initialized" << std::endl;
    this->xbee_device = fd;

    return(fd);
}
