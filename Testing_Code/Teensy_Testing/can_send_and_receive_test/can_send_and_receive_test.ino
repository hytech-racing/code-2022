/*
 * Test program that sends and receives CAN messages. Use this to test if CAN Bus is operational.
 * Created by Nathan Cheek, December 20, 2017
 */
#include <FlexCAN.h>
#include <Metro.h>

FlexCAN CAN(500000);
CAN_message_t msg;
Metro timer_can = Metro(1000);
Metro timer_light = Metro(3);

void setup() {
    Serial.begin(115200); // Initialize serial for PC communication
    CAN.begin();
    delay(200);
    Serial.println("CAN transceiver initialized");
    Serial.println("CAN TEST SENDER/RECEIVER");
    pinMode(13, OUTPUT);
}

void loop() {
    if (timer_can.check()) { // Send a message on CAN
        uint32_t t = millis();
        msg.id = 0x1;
        msg.len = sizeof(uint32_t);
        memcpy(msg.buf, &t, sizeof(uint32_t));
        CAN.write(msg);
        Serial.print("Sent 0x");
        Serial.print(msg.id, HEX);
        Serial.print(": ");
        for (unsigned int i = 0; i < msg.len; i++) {
            Serial.print(msg.buf[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        digitalWrite(13, HIGH);
        timer_light.reset();
    }
    while (CAN.read(msg)) { // Receive a message on CAN
        Serial.print("Received 0x");
        Serial.print(msg.id, HEX);
        Serial.print(": ");
        for (unsigned int i = 0; i < msg.len; i++) {
            Serial.print(msg.buf[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        digitalWrite(13, HIGH);
        timer_light.reset();
    }
    if (timer_light.check()) { // Turn off LED
        digitalWrite(13, LOW);
    }
}
