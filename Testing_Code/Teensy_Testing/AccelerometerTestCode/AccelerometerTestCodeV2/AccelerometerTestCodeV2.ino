#include <SPI.h>

const long X_MSB = 0x1500;
const long X_LSB = 0x1000;
const byte Y_MSB = 0x1C;
const byte Y_LSB = 0x19;
const long Z_MSB = 0x2500;
const long Z_LSB = 0x2000;
const long TEMP_MSB = 0x4C00;
const long TEMP_LSB = 0x4900;

const byte read_CTRL = 0x04;
const byte read_STATUS = 0x08;
const byte read_ID = 0x9D;
const byte read_INTSTATUS = 0x58;

const byte write_CTRLSTS = 0x0704; //CTRL Start memory self-test

const byte RESET1 = 0x0C;
const byte RESET2 = 0x05;
const byte RESET3 = 0x0F;

const int CS = 10;


byte byteIn;

void setup() {
  // put your setup code here, to run once:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  Serial.begin(9600);
  SPI.begin();


  //Start up Sequence
  delay(1000);
  Serial.println("Hello World");
  Serial.println(spiMsg(read_INTSTATUS), BIN);
  Serial.println(spiMsg(write_CTRLSTS), BIN);
  delay(10);
  Serial.println(spiMsg(read_CTRL), BIN);
  Serial.println();
  Serial.println();
  Serial.println();
  delay(3000);
}

void loop() {
  Serial.print(spiMsg(X_MSB), HEX);
  Serial.print(" ");
  Serial.println(spiMsg(X_LSB), HEX);
  
  delay(1000);

}

long spiMsg(long msg) {
  long byteOut;  
  digitalWrite(CS, LOW);
  byteOut = SPI.transfer16(msg);
  digitalWrite(CS, HIGH);
  return byteOut;
}

