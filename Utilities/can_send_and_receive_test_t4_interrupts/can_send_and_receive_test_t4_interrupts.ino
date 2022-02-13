/*
 * Test program that sends and receives CAN messages. Use this to test if CAN Bus is operational.
 * Created by Nathan Cheek, December 20, 2017
 */
// #include <IFCT.h>
#include <HyTech_FlexCAN_T4>
#include <Metro.h>

// FlexCAN CAN(500000);
FlexCAN_T4<CAN1> CAN;
CAN_message_t msg;
Metro timer_can = Metro(1000);
Metro timer_light = Metro(3);

void receive(CAN_message_t &msg) {
    // while (CAN.read(msg)) {
        Serial.print("Received 0x");
        Serial.print(msg.id, HEX);
        Serial.print(": ");
        for (unsigned int i = 0; i < msg.len; i++) {
            Serial.print(msg.buf[i]);
            Serial.print(" ");
        }
        Serial.println();
        digitalWrite(13, HIGH);
        timer_light.reset();
    // }
}

void setup() {
    Serial.begin(115200); // Initialize serial for PC communication
    CAN.begin();
    CAN.setBaudRate(500000);

    CAN.enableMBInterrupts();
    CAN.onReceive(receive);

    delay(200);
    Serial.println("CAN transceiver initialized");
    Serial.println("CAN TEST SENDER/RECEIVER");
    pinMode(13, OUTPUT);
    // CAN0_MCR &= 0xFFFDFFFF; // Enable self-reception
}

void loop() {
    CAN.events();

    if (timer_can.check()) { // Send a message on CAN
        uint32_t t = millis();
        msg.id = 0x2;
        msg.len = sizeof(uint32_t);
        memcpy(msg.buf, &t, sizeof(uint32_t));
        CAN.write(msg);
        Serial.print("Sent 0x");
        Serial.print(msg.id, HEX);
        Serial.print(": ");
        for (unsigned int i = 0; i < msg.len; i++) {
            Serial.print(msg.buf[i]);
            Serial.print(" ");
        }
        Serial.println();
    }

    if (timer_light.check()) { // Turn off LED
        digitalWrite(13, LOW);
    }
}
