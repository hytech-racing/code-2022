/*
   Test program that sends and receives CAN messages. Use this to test if CAN Bus is operational.
   Created by Nathan Cheek, December 20, 2017
*/
#include <mcp_can.h>
#include <Metro.h>

#define CAN_CS 10

// CAN Variables
Metro timer_can_update = Metro(100);
MCP_CAN CAN(CAN_CS);

Metro timer_can = Metro(1000);
Metro timer_light = Metro(3);

void setup() {
  //Initializes CAN
  while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 250K
  {
    delay(200);
  }
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}

void loop() {
  read_can();
}

inline void read_can() {
  //len is message length, buf is the actual data from the CAN message
  static unsigned char len = 0;
  static unsigned char buf[8] = {0};
//  for (int i = 0; i < 4; i++) {
//    buf[i] = i;
//  }
//
  while (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);
    static unsigned long canID = {};
    canID = CAN.getCanId();
    if (canID == 0x1) {
      digitalWrite(3, HIGH);
    }
  }
//  // delay(1000);
//   CAN.sendMsgBuf(0x1, 0, 4, buf);
//   //delay(1000);

}
