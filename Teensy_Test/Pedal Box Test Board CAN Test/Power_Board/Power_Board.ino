/**
 * Nathan Cheek
 * 2016-10-21
 * Read data from Pedal Box test board and turn on and off solid state relays
 */

#include <FlexCAN.h>
#include <Metro.h>

FlexCAN CAN(500000);
static CAN_message_t msg;
long msTimer = 0;

Metro ssrEngage = Metro(5000);
boolean ssrEngaged = false;

void setup() {
  Serial.begin(115200); //initialize serial for PC communication

  CAN.begin();
  delay(1000);
  Serial.println("CAN transceiver initialized");

  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
  
}

void loop() {
  /*
   * Handle incoming CAN messages
   */
  while (CAN.read(msg)) {
    Serial.print(msg.id);
    Serial.print(": ");
    
    for (unsigned int i = 0; i < msg.len; i += 2) {
      int value = msg.buf[i] + (msg.buf[i+1] << 8);
      float voltage = (float)value / 155.1515;
      Serial.print(voltage);
      Serial.print("V ");
    }
    Serial.print(millis());
    Serial.println();
  }

  /*
   * Periodically turn on and off solid state relays
   */
  if (ssrEngage.check()) {
    if (ssrEngaged) {
      ssrEngaged = false;
      digitalWrite(10, LOW);
      digitalWrite(11, LOW);
      digitalWrite(13, LOW);
    } else {
      ssrEngaged = true;
      digitalWrite(10, HIGH);
      digitalWrite(11, HIGH);
      digitalWrite(13, HIGH);
    }
  }
}
