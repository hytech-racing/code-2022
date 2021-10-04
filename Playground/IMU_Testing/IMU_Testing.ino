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

    while (CAN.begin(CAN_500KBPS) != CAN_OK) {
        Serial.println("CAN FAIL!");
        delay(100);
    }
    Serial.println("CAN OK!");
}

void loop() {
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        CAN.readMsgBuf(&len, buf);

        unsigned long canId = CAN.getCanId();

        // accelerometer
        if (canId == BASE_ID) {
            int16_t lat_accel = ((int16_t)(buf[1]) << 8) | buf[0];
            int16_t long_accel = ((int16_t)(buf[3]) << 8) | buf[2];
            int16_t vert_accel = ((int16_t)(buf[5]) << 8) | buf[4];

            Serial.println("-----------------------------");
            Serial.print("Lateral:\t");
            Serial.println(lat_accel);
            Serial.print("Longitudinal:\t");
            Serial.println(long_accel);
            Serial.print("Vertical:\t");
            Serial.println(vert_accel);
            Serial.println();
        }

        // gyroscope
        if (canId == BASE_ID+1) {
            // multiply by 360 to get degrees
            int16_t yaw = (((int16_t)(buf[1]) << 8) | buf[0]) * 360;
            int16_t pitch = (((int16_t)(buf[3]) << 8) | buf[2]) * 360;
            int16_t roll = (((int16_t)(buf[5]) << 8) | buf[4]) * 360;

            Serial.println("-----------------------------");
            Serial.print("Yaw:\t");
            Serial.println(yaw);
            Serial.print("Pitch:\t");
            Serial.println(pitch);
            Serial.print("Roll:\t");
            Serial.println(roll);
            Serial.println();
        }
    }
}