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
    Wire.begin();
    CAN.begin();

    Serial.println("Setup Complete");
}

void loop() {

}
