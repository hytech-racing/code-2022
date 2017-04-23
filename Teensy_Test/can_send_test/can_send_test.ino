#include <FlexCAN.h>
#include <Metro.h>

FlexCAN CAN(500000);
CAN_message_t msg;
Metro timer_can = Metro(10);
Metro timer_light = Metro(3);
int num = 0;

void setup() {
  Serial.begin(115200); //initialize serial for PC communication
  CAN.begin();
  delay(200);
  Serial.println("CAN transceiver initialized");
  Serial.println("CAN TEST SENDER");
  pinMode(13, OUTPUT);
  msg.id = 0x1;
  msg.len = 8;
}

void loop() {
  if (timer_can.check()) { // Send a message on CAN
    memcpy(msg.buf, &num, sizeof(uint32_t));
    CAN.write(msg);
    Serial.print("Sent 0x");
    Serial.print(msg.id, HEX);
    Serial.print(": ");
    for (unsigned int i = 0; i < msg.len; i++) {
      Serial.print(msg.buf[i]);
      Serial.print(" ");
    }
    Serial.println();
    digitalWrite(13, HIGH);
    timer_light.reset();
    if (num < 100) num++;
    else num = 0;
  }
  if (timer_light.check()) { // Turn off LED
    digitalWrite(13, LOW);
  }
}
