#include "mcp_can.h"
#include <SPI.h>

#define CAN_INT 2
#define BASE_ID 0x470
MCP_CAN CAN(9);

unsigned char len = 0;
unsigned char buf[8];

void setup() {
    Serial.begin(115200);
    while (!Serial);

    while (CAN.begin(CAN_1000KBPS) != CAN_OK) {
        Serial.println("CAN FAIL!");
        delay(100);
    }
    Serial.println("CAN OK!");

    unsigned char zero_buf[8] = {0x69, 0x6d, 0x75, 0x7a, 0x65, 0x72, 0x6f, 0x7a};
    CAN.sendMsgBuf(0x10, 0, 8, zero_buf);
}

void loop() {
  Serial.println(CAN.checkReceive());
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        CAN.readMsgBuf(&len, buf);

        unsigned long canId = CAN.getCanId();

        // accelerometer
        if (canId == BASE_ID) {
            int16_t lat_accel = ((int16_t)(buf[0]) << 8) | buf[1];
            int16_t long_accel = ((int16_t)(buf[2]) << 8) | buf[3];
            int16_t vert_accel = ((int16_t)(buf[4]) << 8) | buf[5];

            Serial.println("-----------------------------");
            Serial.print("Lateral:\t");
            Serial.println(lat_accel / 1000. * 9.813);
            Serial.print("Longitudinal:\t");
            Serial.println(long_accel / 1000. * 9.813);
            Serial.print("Vertical:\t");
            Serial.println(vert_accel / 1000. * 9.813);
            Serial.println();
        }

        // gyroscope
        if (canId == BASE_ID+1) {
            // multiply by 360 to get degrees
            int16_t yaw = (((int16_t)(buf[0]) << 8) | buf[1]) * 360;
            int16_t pitch = (((int16_t)(buf[2]) << 8) | buf[3]) * 360;
            int16_t roll = (((int16_t)(buf[4]) << 8) | buf[5]) * 360;

            Serial.println("-----------------------------");
            Serial.print("Yaw:\t");
            Serial.println(yaw / 1000.);
            Serial.print("Pitch:\t");
            Serial.println(pitch / 1000.);
            Serial.print("Roll:\t");
            Serial.println(roll / 1000.);
            Serial.println();
        }

        delay(500);
    }
}
