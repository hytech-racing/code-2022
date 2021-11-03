/*
   Sensor Acquisition Board Ports:                    Teensy 3.2 Pin Number:
   1: Front-Left Suspension Linear Potentiometer      A0
   2: Front-Right Suspension Linear Potentiometer     A1
   3: Steering Wheel Sensor                           A2
   4: Ambient Air Humidity                            A3
   5: Back-Left Suspension Linear Potentiometer       A4
   6: Back-Right Suspension Linear Potentiometer      A5
   7: Ambient Air Temperature                         A6
   8: Motor Loop Cooling Fluid Temperature            A7
*/

#include "CAN_ID.h"
#include "mcp_can.h"
#include <SPI.h>

#define CAN_INT 2
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

    if (canId == ID_SAB_FRONT) {
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
    } else if (canId == ID_SAB_REAR) {
      int16_t sensor_5 = ((int16_t)(buf[1]) << 8) | buf[0];
      int16_t sensor_6 = ((int16_t)(buf[3]) << 8) | buf[2];
      int16_t sensor_7 = ((int16_t)(buf[5]) << 8) | buf[4];
      int16_t sensor_8 = ((int16_t)(buf[7]) << 8) | buf[6];

      Serial.println("-----------------------------");
      Serial.print("Sensor 5:\t");
      Serial.println(sensor_5);
      Serial.print("Sensor 6:\t");
      Serial.println(sensor_6);
      Serial.print("Sensor 7:\t");
      Serial.println(sensor_7);
      Serial.print("Sensor 8:\t");
      Serial.println(sensor_8);
      Serial.println();

    }

    delay(500);
  }
}
