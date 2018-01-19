// include the SPI library:
#include <SPI.h>
#define ADC_SPI_SCK 13
#define ADC_SPI_CS 9
#define ADC_SPI_DOUT 11
#define ADC_SPI_DIN 12

void setup() {
  // set the slaveSelectPin as an output:
  pinMode (ADC_SPI_CS, OUTPUT);
  pinMode (ADC_SPI_CS, HIGH);

  pinMode(ADC_SPI_SCK, OUTPUT);
  pinMode(ADC_SPI_DOUT, OUTPUT);
  pinMode(ADC_SPI_DIN, INPUT);

  // initialize SPI:
  SPI.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(read_adc(0));
  Serial.print("\t");
  Serial.print(read_adc(1));
  Serial.print("\t");
  Serial.print(read_adc(2));
  Serial.print("\t");
  Serial.print(read_adc(3));
  Serial.print("\t");
  Serial.print(read_adc(4));
  Serial.println(" ");
  delay(10);
}

/*
 * Read analog value from MCP3208 ADC
 * http://ww1.microchip.com/downloads/en/DeviceDoc/21298c.pdf
 * Page 15 ~ 18
 */
uint16_t read_adc(int channel) {
  // gain control of the SPI port
  // and configure settings
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  
  // take the SS pin low to select the chip:
  digitalWrite(ADC_SPI_CS,LOW);
  
  // set up channel
  byte b = B01100000; 
  b |= ((channel<<2));
  
  //send in the channel via SPI:
  SPI.transfer(b);

  // read data from SPI
  byte result1 = SPI.transfer(0);
  byte result2 = SPI.transfer(0);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(ADC_SPI_CS,HIGH);
  
  // release control of the SPI port
  SPI.endTransaction();

  return (result1 << 4) | (result2 >> 4);
}



