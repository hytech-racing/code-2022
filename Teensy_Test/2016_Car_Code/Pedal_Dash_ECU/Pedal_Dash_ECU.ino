/**
 * Nathan Cheek
 * 2016-11-12
 * Interface with dashboard lights, buttons, and buzzer. Read pedal sensor values.
 */
#include <FlexCAN.h>
#include <Metro.h>

/*
 * Pin definitions
 */
int BTN_TOGGLE = 9;
int BTN_CYCLE = 10;
int BTN_BOOST = 11;
int BTN_START = 12;
int LED_START = 7;
int LED_IMD = 6;
int LED_BMS = 5;
int READY_SOUND = 8;
int PEDAL_SIGNAL_A = A3;
int PEDAL_SIGNAL_B = A4;
int PEDAL_SIGNAL_C = A5;

/*
 * CAN setup
 */
FlexCAN CAN(500000);
static CAN_message_t msg;
uint16_t STATE_MESSAGE_ID = 0xD0;
uint16_t ENABLE_LED_START = 0xD1;
uint16_t DISABLE_LED_START = 0xD2;
uint16_t ENABLE_LED_BMS = 0xD3;
uint16_t DISABLE_LED_BMS = 0xD4;
uint16_t ENABLE_LED_IMD = 0xD5;
uint16_t DISABLE_LED_IMD = 0xD6;
uint16_t BLINK_LED_START = 0xD7;

/*
 * Timers
 */
Metro timer_printer = Metro(500);
Metro timer_state_send = Metro(50);
Metro timer_motor_controller_send = Metro(50);
Metro timer_led_start_blink = Metro(300);

/*
 * State booleans
 */
boolean STATE_LED_START = false;
boolean STATE_LED_IMD = false;
boolean STATE_LED_BMS = false;
boolean STATE_READY_SOUND = false;
boolean STATE_LED_START_BLINK = false;

void setup() {
  pinMode(BTN_TOGGLE, INPUT_PULLUP);
  pinMode(BTN_CYCLE, INPUT_PULLUP);
  pinMode(BTN_BOOST, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(LED_START, OUTPUT);
  pinMode(LED_IMD, OUTPUT);
  pinMode(LED_BMS, OUTPUT);
  pinMode(READY_SOUND, OUTPUT);

  CAN.begin();
  delay(1000);
  Serial.println("CAN transceiver initialized");
  Serial.begin(115200);
}

void loop() {
  /*
   * Send state over CAN
   */
  if (timer_state_send.check()) {
    msg.id = STATE_MESSAGE_ID;
    msg.len = 1;
    msg.buf[0] = !digitalRead(BTN_TOGGLE) << 7 | !digitalRead(BTN_CYCLE) << 6 | !digitalRead(BTN_BOOST) << 5 | !digitalRead(BTN_START) << 4;
    msg.buf[0] |= STATE_LED_START << 3 | STATE_LED_IMD << 2 | STATE_LED_BMS << 1 | STATE_READY_SOUND;
    CAN.write(msg);
  }

  /*
   * Send a message to the Motor Controller over CAN
   */
  if (timer_motor_controller_send.check()) {
    msg.id = 0xC0;
    msg.len = 8;
    generate_MC_message(msg.buf, 0, 0, 0);
    CAN.write(msg);
  }

  /*
   * Handle incoming CAN messages
   */
  while (CAN.read(msg)) {
    if (msg.id == ENABLE_LED_START) {
      STATE_LED_START = true;
      STATE_LED_START_BLINK = false;
      digitalWrite(LED_START, HIGH);
      Serial.println("Start LED Enabled");
    } else if (msg.id == DISABLE_LED_START) {
      STATE_LED_START = false;
      STATE_LED_START_BLINK = false;
      digitalWrite(LED_START, LOW);
      Serial.println("Start LED Disabled");
    } else if (msg.id == BLINK_LED_START && !STATE_LED_START_BLINK) {
      STATE_LED_START = true;
      STATE_LED_START_BLINK = true;
      digitalWrite(LED_START, HIGH);
      timer_led_start_blink.reset();
      Serial.println("Start LED Blink");
    } else if (msg.id == ENABLE_LED_BMS) {
      STATE_LED_BMS = true;
      digitalWrite(LED_BMS, HIGH);
    } else if (msg.id == DISABLE_LED_BMS) {
      STATE_LED_BMS = false;
      digitalWrite(LED_BMS, LOW);
    } else if (msg.id == ENABLE_LED_IMD) {
      STATE_LED_IMD = true;
      digitalWrite(LED_IMD, HIGH);
    } else if (msg.id == DISABLE_LED_IMD) {
      STATE_LED_IMD = false;
      digitalWrite(LED_IMD, LOW);
    }
    if (msg.id > 0xBF) {
      Serial.print(msg.id, HEX);
      Serial.print(": ");
      for (unsigned int i = 0; i < msg.len; i++) {
        Serial.print(msg.buf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
  }

  if (timer_printer.check()) {
    /*Serial.print(analogRead(PEDAL_SIGNAL_A));
    Serial.print(" ");
    Serial.print(analogRead(PEDAL_SIGNAL_B));
    Serial.print(" ");
    Serial.println(analogRead(PEDAL_SIGNAL_C));
    Serial.println(analogRead(BTN_START));*/
  }

  if (timer_led_start_blink.check() && STATE_LED_START_BLINK) {
    if (STATE_LED_START) {
      digitalWrite(LED_START, LOW);
    } else {
      digitalWrite(LED_START, HIGH);
    }
    STATE_LED_START = !STATE_LED_START;
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
