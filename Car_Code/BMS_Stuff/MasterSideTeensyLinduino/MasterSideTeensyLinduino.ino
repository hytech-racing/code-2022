/**
 * Created by Shrivathsav Seshan
 * Teensy (master) - Linduino (slave) communication code, master side.
 * 4/2/2017
 */

#include <SPI.h>

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    SPI.begin();
    pinMode(10, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
    char* incoming;
    if (Serial.available()) {
        while (Serial.available()) {
            char c = Serial.read();
            writeChar(c);
        }
    }
    delay(10); // delay 10 milliseconds
    readIncoming();
}

void writeChar(char c) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(10, LOW);
    SPI.transfer(c);
    digitalWrite(10, HIGH);
    SPI.endTransaction();
}

void writeString(char* myString) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    boolean nullFound = false;
    int i = 0;
    digitalWrite(10, LOW);
    while (!nullFound) {
        byte c = (byte) myString[i];
        if (c == '\0') {
            nullFound = true;
        }
        SPI.transfer(c);
        i++;
    }
    digitalWrite(10, HIGH);
    SPI.endTransaction();
}

void readIncoming() {
    digitalWrite(10, LOW);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    boolean nullFound = false;
    int i = 0;
    while (!nullFound) {
        char c = (char) SPI.transfer(255);
        if (c == '\0') {
            nullFound = true;
        } else {
            Serial.print(c);
        }
        i++;
    }
    SPI.endTransaction();
    digitalWrite(10, HIGH);
}

