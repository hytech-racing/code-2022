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

bool readCAN(); // Reads CAN message for controlling BMS
bool processCAN(); // processes and sends I2C request, called within
bool writeI2C(int configID); // transmits data to linduino

bool requestI2C(int configID); // sends I2C request for linduino
bool processI2C(); // processes and sends CAN message, called within
bool writeCAN(); // writes status messages for BMS onto CAN

void setup() {
    Serial.begin(115200);
    delay(1000);
    Wire.begin();
    delay(1000);
    CAN.begin();

    Serial.println("Setup Complete");
}

void loop() {
    Wire.requestFrom(1, 32);
    int index = 0;
    uint8_t buf[8];
    while (Wire.available() && index < 32) {
        if (index == 8) {
            msg.id = ID_BMS_VOLTAGE;
            msg.buf = buf;
            msg.len = 8;
            CAN.write(msg);
        } else if (index == 16) {
            msg.id = ID_BMS_CURRENT;
            msg.buf = buf;
            msg.len = 8;
            CAN.write(msg);
        } else if (index == 24) {
            msg.id = ID_BMS_TEMPERATURE;
            msg.buf = buf;
            msg.len = 8;
            CAN.write(msg);
        } else if (index == 32) {
            msg.id = ID_BMS_STATUS;
            msg.buf = buf;
            msg.len = 8;
            CAN.write(msg);
        }
        buf[index % 8] = b;
        index++;
    }
}
