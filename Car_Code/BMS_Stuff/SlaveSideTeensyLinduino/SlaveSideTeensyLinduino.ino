/**
 * Created by Shrivathsav Seshan
 * Teensy (master) - Linduino (slave) communication code, slave side.
 * 4/2/2017
 */

#include <SPI.h>

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    SPI.begin();
    pinMode(10, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
    
}

void readResponse() {
    
}

