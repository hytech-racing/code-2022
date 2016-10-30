#include <FlexCAN.h>
#include <Metro.h>

FlexCAN CAN(500000);
static CAN_message_t msg;
unsigned char buf[8];

Metro MCStatusMessage = Metro(100);
Metro latchEnable = Metro(10000);
Metro latchTimer = Metro(1000);

boolean hasLatched = false;
boolean isLatching = false;



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
    Print any CAN messages received
  */
  while (CAN.read(msg)) {
    Serial.print(msg.id, HEX);
    Serial.print(": ");
    for (unsigned int i = 0; i < msg.len; i++) {
      Serial.print(msg.buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }

  /*
   * Send a message to the Motor Controller
   */
  if (MCStatusMessage.check()) {
    msg.id = 0xC0;
    msg.len = 8;
    generate_MC_message(msg.buf, 0, 0, 0);
    CAN.write(msg);
  }

  /*
   * Latch the tractive system automatically after 20 seconds
   */
  if (!hasLatched && latchEnable.check()) {
    isLatching = true;
    latchTimer.reset();
    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);
  }

  if (isLatching && latchTimer.check()) {
    isLatching = false;
    hasLatched = true;
    digitalWrite(10, LOW);
  }

  
}

void generate_MC_message(unsigned char* message, int thatTorque, boolean backwards, boolean enable) {
  message[0] = thatTorque & 0xFF;
  message[1] = thatTorque>>(8);
  message[2] = 0;
  message[3] = 0;
  message[4] = char(backwards);
  message[5] = char(enable);
  message[6] = 0;
  message[7] = 0;
}
