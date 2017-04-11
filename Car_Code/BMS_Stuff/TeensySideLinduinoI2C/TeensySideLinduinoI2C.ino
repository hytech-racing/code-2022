/*
  TeensySideLinduinoI2C.ino - Code that runs on BMS Teensy. Serves as a relay between the Linduino and the Car CAN system
  Created by Charith "Karvin" Dassanayake & Shrivathsav Seshan, April 11, 2017.
 */

#include <Wire.h>
#include <Arduino.h>
#include <stdint.h>
#include <FlexCAN.h>
#include <HyTech17.h>

/**
 * BMS State Variables
 */
BMS_voltages bmsVoltageMessage;
BMS_currents bmsCurrentMessage;
BMS_temperatures bmsTempMessage;
BMS_status bmsStatusMessage;

// BMS_voltages data static
// BMS_currents data static
// BMS_temperatures data static
// BMS_status data static

FlexCAN CAN(500000);
static CAN_message_t msg;

// bool readCAN(); // Reads CAN message for controlling BMS
// bool processCAN(); // processes and sends I2C request, called within
// bool writeI2C(int configID); // transmits data to linduino
//
// bool requestI2C(int configID); // sends I2C request for linduino
// bool processI2C(); // processes and sends CAN message, called within
// bool writeCAN(); // writes status messages for BMS onto CAN

static int runningIndex; // Used for tracking
uint8_t buf[8];

void setup() {
    Serial.begin(115200);
    delay(1000);
    Wire.begin();
    delay(1000);
    CAN.begin();

    runningIndex = 0;
    Serial.println("Setup Complete");
}

void loop() {
    Wire.requestFrom(1, 32);

    while (Wire.available() && runningIndex < 32) {

         if (runningIndex % 8 == 0) {
            memcpy(&(msg.buf[0]), &buf[0], 8 * sizeof(uint8_t));
         }

        if (runningIndex == 8) {
            msg.id = ID_BMS_VOLTAGE;
            // msg.buf = buf;
            msg.len = 8;
            CAN.write(msg);
        } else if (runningIndex == 16) {
            msg.id = ID_BMS_CURRENT;
            // msg.buf = buf;
            msg.len = 8;
            CAN.write(msg);
        } else if (runningIndex == 24) {
            msg.id = ID_BMS_TEMPERATURE;
            // msg.buf = buf;
            msg.len = 8;
            CAN.write(msg);
        } else if (runningIndex == 32) {
            msg.id = ID_BMS_STATUS;
            // msg.buf = buf;
            msg.len = 8;
            CAN.write(msg);
        }
        int tempNum = runningIndex % 32;
        runningIndex = tempNum;

        char readingChar = Wire.read();
        Serial.println(readingChar);
        buf[runningIndex % 8] = readingChar;
        runningIndex++;
    }
}
