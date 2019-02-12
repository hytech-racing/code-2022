#include <SPI.h>
#include <SparkFun_TB6612.h>
#include "mcp_can.h"

#define AIN1 2
#define BIN1 7
#define AIN2 4
#define BIN2 8
#define PWMA 5
#define PWMB 6
#define STBY 9

const int offsetA = 1;
const int offsetB = 1;

Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

long unsigned int rxId;

unsigned long rcvTime;

unsigned char len = 0;
unsigned char buf[8];


const int SPI_CS_PIN = 10;


MCP_CAN CAN(SPI_CS_PIN);

void setup()
{
  Serial.begin(115200);


  while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Module Failed to Initialized");
    Serial.println("Retrying....");
    delay(200);
  }
  Serial.println("CAN BUS Module Initialized!");
  Serial.println("Time\t\tPGN\t\tByte0\tByte1\tByte2\tByte3\tByte4\tByte5\tByte6\tByte7");
}


void loop()
{
  if (CAN_MSGAVAIL == CAN.checkReceive())           // check if data coming
  {
    rcvTime = millis();
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

    rxId = CAN.getCanId();

    Serial.print(rcvTime);
    Serial.print("\t\t");
    Serial.print("0x");
    Serial.print(rxId, HEX);
    Serial.print("\t");

    for (int i = 0; i < len; i++) // print the data
    {
      if (buf[i] > 15) {
        Serial.print("0x");
        Serial.print(buf[i], HEX);
      }
      else {
        Serial.print("0x0");
        Serial.print(buf[i], HEX);
      }

      //Serial.print("0x");
      //Serial.print(buf[i], HEX);

      Serial.print("\t");
    }
    Serial.println();
    switch (buf[0]) {
      case 0x00:
        Serial.println("forward");
        motor1.drive(255, 200);
        motor2.drive(255, 200);
        motor1.brake();
        motor2.brake();
        break;
      case 0x01:
        Serial.println("left");
        motor1.drive(-255, 200);
        motor2.drive(255, 200);
        motor1.brake();
        motor2.brake();
        break;
      case 0x02:
        Serial.println("right");
        motor1.drive(255, 200);
        motor2.drive(-255, 200);
        motor1.brake();
        motor2.brake();
        break;
      case 0x03:
        Serial.println("backwards");
        motor1.drive(-255, 200);
        motor2.drive(-255, 200);
        motor1.brake();
        motor2.brake();
        break;
      default:
        Serial.println("unrecognised command");
    }

  }
}
