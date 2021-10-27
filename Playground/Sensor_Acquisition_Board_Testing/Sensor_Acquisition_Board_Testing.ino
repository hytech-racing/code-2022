#include "mcp_can.h"
#include <SPI.h>

#define CAN_INT 2
#define BASE_ID 0xEC
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

        if (canId == BASE_ID) {
            int16_t sensor_1 = ((int16_t)(buf[1]) << 8) | buf[0];
            int16_t sensor_2 = ((int16_t)(buf[3]) << 8) | buf[2];
            int16_t sensor_3 = ((int16_t)(buf[5]) << 8) | buf[4];
            int16_t sensor_4 = ((int16_t)(buf[7]) << 8) | buf[6];

            Serial.println("-----------------------------");
            Serial.print("Sensor 1:\t");
            Serial.println(sensor_1);
            Serial.print("Sensor 2:\t");
            Serial.println(sensor_2);
            Serial.print("Sensor 3:\t");
            Serial.println(sensor_3);
            Serial.print("Sensor 4:\t");
            Serial.println(sensor_4);
            Serial.println();
        }

        delay(500);
    }
}
