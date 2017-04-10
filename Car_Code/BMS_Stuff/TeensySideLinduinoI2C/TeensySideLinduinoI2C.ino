/**
 * Created by Shrivathsav Seshan
 * Teensy (master) - Linduino (slave) serial communication code.
 */

#include <Wire.h>
#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include <FlexCAN.h>
#include "HyTech17.h"

// TODO: Integrate with CAN Receiving
//        - Read from BUS.
//        - Populate Library Objects as needed (What does BMS respond to anyway?)
// TODO: Integrate with CAN Transmitting
//        - Create protocol or API for Teensy to Linduino Communication to expose all Linduino BMS functionality.
//        - Write configuration registers
//        - Read configuration registers
//        - Initialize main ADC conversion
//        - Read cell registers
//        - Initialize auxiliary ADC conversion
//        - Read auxiliary registers
//        - Read status registers
//        - Initialize Discharging and Balancing (writing to configuration registers)


/**
 * BMS State Variables
 */
BMS_voltages bmsVoltageMessage;
BMS_currents bmsCurrentMessage;
BMS_temperatures bmsTempMessage;
BMS_status bmsStatusMessage;

/**
 * CAN Variables
 */
FlexCAN can(500000);
static CAN_message_t msg;
long msTimer = 0;

void setup() {
    // adfads
    Serial.begin(115200);
    delay(1000);
    Wire.begin();
    delay(1000);
    can.begin();
    Serial.println("Setup Complete!");
}

void loop() {
    // Repeated code goes in here.
    while (!Serial.available()) {
        // wait for incoming data in Serial.
    }
    if (Serial.available()) {
        while (Serial.available()) {
            char c = Serial.read();
            if (i < 50) {
                myString[i] = c;
                i++;
            }
        }
    }
    Wire.beginTransmission(1);
    for (int j = 0; j < i; j++) {
        Wire.write((byte) myString[j]);
    }
    Wire.endTransmission();
    i = 0;
}
