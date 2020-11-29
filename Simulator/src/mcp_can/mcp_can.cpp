
#include "Arduino.h"
#include "CAN_simulator.h"
#include "mcp_can.h"

MCP_CAN::MCP_CAN(byte pin) {
    SPICS = pin;
}

byte MCP_CAN::begin(byte speed) {
    if (speed != CAN_500KBPS)
        throw CustomException("CAN must use 500 kpbs baud rate");
    pinMode(SPICS, RESERVED);
    filhit = true;
    return 0;
}

byte MCP_CAN::checkReceive(void) {
    if (!filhit)
        throw CustomException("CAN config not valid");
    return CAN_simulator::vehicle_inbox.size() ? CAN_MSGAVAIL : CAN_NOMSG;
}

unsigned long MCP_CAN::getCanId(void) { return can_id; }

byte MCP_CAN::sendMsgBuf(unsigned long id, byte ext, byte len, byte *buf) {
    if (!filhit)
        throw CustomException("CAN config not valid");
    CAN_message_t msg;
    msg.id = id;
    msg.ext = ext;
    msg.len = len;
    memcpy(msg.buf, buf, len);
    CAN_simulator::vehicle_write(msg);
    return true;
}

byte MCP_CAN::readMsgBuf(byte *len, byte *buf) {
    CAN_message_t msg;
    if (!CAN_simulator::vehicle_read(msg))
        throw CustomException("CAN buffer is empty");
    can_id = msg.id;
    ext_flg = msg.ext;
    rtr = msg.rtr;
    dta_len = msg.len;
    memcpy(dta, msg.buf, dta_len);

    *len = dta_len;
    memcpy(buf, dta, dta_len);
    return true;
}
