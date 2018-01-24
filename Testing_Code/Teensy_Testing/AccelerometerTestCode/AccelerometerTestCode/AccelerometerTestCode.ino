#include <SPI.h>

const byte read_ACCX[] = {0x04, 0x00, 0x00, 0xF7};
const long read_ACCY = 0x080000FD;
const long read_ACCZ = 0x0C0000FB;
const byte read_STO[] = {0x10, 0x00, 0x00, 0xE9};
const byte read_TEMP[] = {0x14, 0x00, 0x00, 0xEF};
const long read_SS = 0x180000E5;
const long RESETSW = 0xB4002098;
const byte CHMODE1[] = {0xB4, 0x00, 0x00, 0x1F};
const byte CHMODE2[] = {0xB4, 0x00, 0x01, 0x02};
const long CHMODE3 = 0xB4000225;
const long CHMODE4 = 0xB4000338;
const byte read_WAI[] = {0x40, 0x00, 0x00, 0x91};

const int CS = 10;

byte outData[4];

void setup() {
  // put your setup code here, to run once:
  //pinMode(13, OUTPUT);
  pinMode(CS, OUTPUT);
  Serial.begin(9600);
  SPI.begin();

  spiMsg(CHMODE1, outData);
  delay(5);
  
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  
  spiMsg(read_STO, outData);
  

//  Serial.print(read_[0]);
//  Serial.print(read_WAI[1]);
//  Serial.print(read_WAI[2]);
//  Serial.println(read_WAI[3]);
  
  Serial.println(outData[0], HEX);
  Serial.println(outData[1], HEX);
  Serial.println(outData[2], HEX);
  Serial.println(outData[3], HEX);
  Serial.println();

  delay(1000);
}

void spiMsg(const byte *fourBytesIn, byte *fourBytesOut) {
  digitalWrite(CS, LOW);
  for (int i = 0; i < 4; i++)
  {
    fourBytesOut[i] = SPI.transfer(fourBytesIn[i]);
  }
  digitalWrite(CS, HIGH);
}

