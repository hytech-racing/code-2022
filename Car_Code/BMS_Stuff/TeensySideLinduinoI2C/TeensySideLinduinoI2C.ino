/*
  TeensySideLinduinoI2C.ino - Code that runs on BMS Teensy. Serves as a relay between the Linduino and the Car CAN system
  Created by Charith "Karvin" Dassanayake & Shrivathsav Seshan, April 11, 2017.
 */

#include <Wire.h>
#include <Arduino.h>
#include <stdint.h>
#include <FlexCAN.h>
#include "HyTech17.h"

/**
 * BMS State Variables
 */
BMS_voltages bmsVoltageMessage;
BMS_currents bmsCurrentMessage;
BMS_temperatures bmsTempMessage;
BMS_status bmsStatusMessage;

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
    Serial.begin(9600);
    delay(2000);
    Wire.begin(8);
    delay(2000);
//    CAN.begin();
    Wire.onReceive(receiveEvent);
    pinMode(13, OUTPUT);
    runningIndex = 0;
    Serial.println("Setup Complete");
}

void loop() {
    digitalWrite(13, HIGH);
}

void receiveEvent(int howMany) {
    while (Wire.available()) {//&& runningIndex < 14) {
        Serial.println("Teensy");
        if (runningIndex % 8 == 0) {
            memcpy(&(msg.buf[0]), &buf[0], 8 * sizeof(uint8_t));
        }
        if (runningIndex == 8) {
            bmsVoltageMessage.load(buf);
            msg.id = ID_BMS_VOLTAGE;
            // msg.buf = buf;
            msg.len = 8;
            Serial.print("Average Voltage: ");
            Serial.println(bmsVoltageMessage.getAverage());
            Serial.print("Total Voltage: ");
            Serial.println(bmsVoltageMessage.getTotal());
//            CAN.write(msg);
        } else if (runningIndex == 16) {
            bmsCurrentMessage.load(buf);
            msg.id = ID_BMS_CURRENT;
            // msg.buf = buf;
            msg.len = 8;
//            CAN.write(msg);
        } else if (runningIndex == 24) {
            bmsTempMessage.load(buf);
            msg.id = ID_BMS_TEMPERATURE;
            // msg.buf = buf;
            msg.len = 8;
//            CAN.write(msg);
        } else if (runningIndex == 32) {
            bmsStatusMessage.load(buf);
            msg.id = ID_BMS_STATUS;
            // msg.buf = buf;
            msg.len = 8;
//            CAN.write(msg);
        }
        int tempNum = runningIndex % 32;
        runningIndex = tempNum;

        byte b = Wire.read();
        buf[runningIndex % 8] = b;
        runningIndex++;
    }
    Serial.print("Total Voltage: "); Serial.println(bmsVoltageMessage.getTotal());
    Serial.print("Average Voltage: "); Serial.println(bmsVoltageMessage.getAverage());
}

