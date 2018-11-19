#include <mcp_can.h>
#include <SPI.h>

#define left_pin      2
#define right_pin     3
#define forward_pin   4
#define backward_pin  5
const int SPI_CS_PIN = 10;


long unsigned int txID = 0x1881ABBA; // This format is typical of a 29 bit identifier.. the most significant digit is never greater than one.
unsigned char f[8] = {0x00, 0x00, 0xFF, 0x22, 0xE9, 0xFA, 0xDD, 0x51};
unsigned char l[8] = {0x01, 0x00, 0xFF, 0x22, 0xE9, 0xFA, 0xDD, 0x51};
unsigned char r[8] = {0x02, 0x00, 0xFF, 0x22, 0xE9, 0xFA, 0xDD, 0x51};
unsigned char b[8] = {0x03, 0x00, 0xFF, 0x22, 0xE9, 0xFA, 0xDD, 0x51};

MCP_CAN CAN(SPI_CS_PIN);                            


void setup()
{
  pinMode(left_pin, INPUT_PULLUP);
  pinMode(right_pin, INPUT_PULLUP);
  pinMode(forward_pin, INPUT_PULLUP);
  pinMode(backward_pin, INPUT_PULLUP);

  
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 250K
    {
         Serial.println("CAN BUS Module Failed to Initialized");
        Serial.println("Retrying....");
        delay(200);
        
    }
    Serial.println("CAN BUS Shield init ok!");
}


void loop()
{   
  int left_val = digitalRead(left_pin);
  int right_val = digitalRead(right_pin);
  int forward_val = digitalRead(forward_pin);
  int backward_val = digitalRead(backward_pin);
  if(forward_val == 0) {
    Serial.println("forward");
    CAN.sendMsgBuf(txID,1, 8, f);    
  } else if(backward_val== 0) {
        Serial.println("backward");
    CAN.sendMsgBuf(txID,1, 8, b); 
  } else if(right_val == 0) {
        Serial.println("right");
    CAN.sendMsgBuf(txID,1, 8, r);  
  } else if(left_val == 0) {
    
    Serial.println("left");
    CAN.sendMsgBuf(txID,1, 8, l); 
  }
   delay(200);
}
