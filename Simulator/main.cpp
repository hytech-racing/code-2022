#include <stdio.h>

#include "Arduino.h"
#include "CAN_simulator.h"
#include "Simulator.h"

int main(int argc, char* argv []) {
    Simulator simulator (100);
    // int reading = 1;
    // while (millis() < 1000) {
    //     io[4].setValue(reading++);
    // }
    CAN_message_t msg;

    msg.id = 0x1;
    msg.len = 8;
    msg.buf[0] = 'A';
    msg.buf[1] = 'B';
    msg.buf[2] = 'C';
    msg.buf[3] = 'D';
    msg.buf[4] = 'E';
    msg.buf[5] = 'F';
    msg.buf[6] = 'G';
    msg.buf[7] = '\0';
    CAN_simulator::write(msg);
    msg.buf[0] = 'H';
    CAN_simulator::write(msg);

    simulator.next();

    while (CAN_simulator::read(msg))
        printf("Received (id %d): %s\n", msg.id, msg.buf);

    return 0;
}