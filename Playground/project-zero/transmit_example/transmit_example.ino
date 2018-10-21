#include <mcp_can.h>
#include <SPI.h>

//INPUT Pin Definitions
#define FORW_PIN  3
#define BACK_PIN  6
#define LEFT_PIN  4
#define RIGHT_PIN  5
#define START_PIN 2

//OUTPUT Pin Definitions
#define POWER_PIN  7
#define STARTED_PIN  9
#define STOPPED_PIN  8

//CAN Messages
unsigned char car_start[2] = {0x20, 0x01};
unsigned char car_stop[2] = {0x20, 0x02};
unsigned char car_forw[2] = {0x10, 0x01};
unsigned char car_back[2] = {0x10, 0x02};
unsigned char car_left[2] = {0x10, 0x03};
unsigned char car_right[2] = {0x10, 0x04};

//CAN and SPI
const int SPI_CS_PIN = 10;
long unsigned int txID = 0x1ABBCC02;
MCP_CAN CAN(SPI_CS_PIN);

//Button States
int start_button = 0;
int forw_button = 0;
int back_button = 0;
int left_button = 0;
int right_button = 0;

//Last Button States
int last_start_button = 0;

//States
bool started = false;

void setup()
{
  Serial.begin(115200);

  while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 250K
  {
    Serial.println("CAN BUS Module Failed to Initialized");
    Serial.println("Retrying....");
    delay(200);

  }
  Serial.println("CAN BUS Shield init ok!");
  pinMode(FORW_PIN, INPUT);
  pinMode(BACK_PIN, INPUT);
  pinMode(LEFT_PIN, INPUT);
  pinMode(RIGHT_PIN, INPUT);
  pinMode(START_PIN, INPUT);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(STARTED_PIN, OUTPUT);
  pinMode(STOPPED_PIN, OUTPUT);

  digitalWrite(POWER_PIN, HIGH);
}


void loop()
{
  //PIN STATES
  start_button = digitalRead(START_PIN);
  forw_button = digitalRead(FORW_PIN);
  back_button = digitalRead(BACK_PIN);
  left_button = digitalRead(LEFT_PIN);
  right_button = digitalRead(RIGHT_PIN);

  //LEDS
  if (started) {
    digitalWrite(STARTED_PIN, HIGH);
    digitalWrite(STOPPED_PIN, LOW);
  } else {
    digitalWrite(STARTED_PIN, LOW);
    digitalWrite(STOPPED_PIN, HIGH);
  }

  //START STOP
  if (start_button == HIGH && start_button != last_start_button) {
    if (started) {
      Serial.println("SENT CAR STOP");
      CAN.sendMsgBuf(txID, 1, 2, car_stop);
      started = false;
    } else {
      Serial.println("SENT CAR START");
      CAN.sendMsgBuf(txID, 1, 2, car_start);
      started = true;
    }
  }

  //CONTROLL
  if (forw_button == HIGH && started) {
    Serial.println("SENT FORWARD");
    CAN.sendMsgBuf(txID, 1, 2, car_forw);
  }
  if (back_button == HIGH && started) {
    Serial.println("SENT BACKWARD");
    CAN.sendMsgBuf(txID, 1, 2, car_back);
  }
  if (left_button == HIGH && started) {
    Serial.println("SENT LEFT");
    CAN.sendMsgBuf(txID, 1, 2, car_left);
  }
  if (right_button == HIGH && started) {
    Serial.println("SENT RIGHT");
    CAN.sendMsgBuf(txID, 1, 2, car_right);
  }
  //Updating Last States
  last_start_button = start_button;

  //Delay
  delay(25);
}
