#include <SPI.h>
#include "mcp_can.h"
#include <SparkFun_TB6612.h>

//TS PIN
#define TS_PIN A4

//MOTOR PINS
#define AIN1 5
#define BIN1 7
#define AIN2 4
#define BIN2 8
#define PWMA 3
#define PWMB 9
#define STBY 6

//CAN VARS
long unsigned int rxId;
unsigned char buf[2];
unsigned char len = 0;

//States
bool started = false;
bool ts_active = false;

//TS button states
int ts_button = 0;
int last_ts_button = 0;

//SPI and CAN
const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

//Motor Init
Motor motor1 = Motor(AIN1, AIN2, PWMA, 1, STBY);
Motor motor2 = Motor(BIN1, BIN2, PWMB, 1, STBY);




void setup()
{
  //CAN
  Serial.begin(115200);
  while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Module Failed to Initialized");
    Serial.println("Retrying....");
    delay(200);
  }
  Serial.println("CAN BUS Module Initialized!");
  Serial.println("Time\t\tPGN\t\tByte0\tByte1\tByte2\tByte3\tByte4\tByte5\tByte6\tByte7");

  //PINS
  pinMode(TS_PIN, INPUT);

  //INITIAL STATES
  Serial.println("NOT STARTED");
  Serial.println("TS NOT ACTIVE");
}


void loop()
{
  //If Message
  if (CAN_MSGAVAIL == CAN.checkReceive())
  {
    //Read Message
    CAN.readMsgBuf(&len, buf);
    rxId = CAN.getCanId();

    //Start stop
    if (buf[0] == 0x20 && buf[1] == 0x01) {
      Serial.print("Start Recieved\n");
      started = true;
    } else if (buf[0] == 0x20 && buf[1] == 0x02) {
      Serial.print("Stop Recieved\n");
      started = false;
    }

    //Directions
    if (started && ts_active) {
      if (buf[0] == 0x10 && buf[1] == 0x01) {
        Serial.print("Forward Recieved\n");
        forward(motor1, motor2, 150);
      } else if (buf[0] == 0x10 && buf[1] == 0x02) {
        Serial.print("Backward Recieved\n");
        back(motor1, motor2, 150);
      } else if (buf[0] == 0x10 && buf[1] == 0x03) {
        Serial.print("Left Recieved\n");
        left(motor1, motor2, 150);
      } else if (buf[0] == 0x10 && buf[1] == 0x04) {
        Serial.print("Right Recieved\n");
        right(motor1, motor2, 150);
      }
    }
  } else {
    brake(motor1, motor2);
  }

  //TS ACTIV BUTTON
  if (started) {
    ts_button = digitalRead(TS_PIN);
    if (ts_button == LOW && ts_button != last_ts_button) {
      if (ts_active) {
        Serial.println("TS NOT ACTIVE");
        brake(motor1, motor2);
        ts_active = false;
      } else {
        Serial.println("TS ACTIVATED");
        ts_active = true;
      }

    }
    last_ts_button = ts_button;
  }
  delay(50);
}
