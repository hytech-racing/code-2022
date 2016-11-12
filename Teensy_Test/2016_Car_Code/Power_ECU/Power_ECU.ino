/**
 * Nathan Cheek
 * 2016-11-12
 * Control Shutdown Circuit initialization.
 */
#include <FlexCAN.h>
#include <Metro.h>

/*
 * Pin definitions
 */
int SSR_BRAKE_LIGHT = 13;
int SSR_SOFTWARE_SHUTOFF = 11;
int SSR_LATCH = 10;
int SENSE_IMD = A0;
int SENSE_BMS = A1;
int SENSE_SHUTDOWN_OUT = A2;
int SENSE_BRAKE = A3;
int SENSE_TEMP = A4;

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
Metro timer_latch = Metro(1000);
Metro timer_send_command = Metro(100);
Metro timer_imd_faulting = Metro(100); // At startup the IMD OKHS line drops shortly
Metro timer_bms_faulting = Metro(100);

/*
 * State booleans
 */
boolean STATE_HAS_LATCHED = false;
boolean STATE_IS_LATCHING = false;
boolean STATE_LED_START = false;
boolean STATE_IMD_BMS_START_MET = false;
boolean STATE_IMD_FAULT = false;
boolean STATE_BMS_FAULT = false;
boolean STATE_IMD_FAULTING = false;
boolean STATE_BMS_FAULTING = false;

uint16_t STATE_IMD_SIGNAL = 0;
uint16_t STATE_BMS_SIGNAL = 0;

void setup() {
  pinMode(SSR_LATCH, OUTPUT);
  pinMode(SSR_SOFTWARE_SHUTOFF, OUTPUT);
  pinMode(SSR_BRAKE_LIGHT, OUTPUT);

  CAN.begin();
  delay(1000);
  Serial.println("CAN transceiver initialized");
  Serial.begin(115200);
}

void loop() {
  /*
   * Handle incoming CAN messages
   */
  while (CAN.read(msg)) {
    if (msg.id > 0xBF) {
      Serial.print(msg.id, HEX);
      Serial.print(": ");
      for (unsigned int i = 0; i < msg.len; i++) {
        Serial.print(msg.buf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }

    if (STATE_IMD_BMS_START_MET && msg.id == STATE_MESSAGE_ID) {
      Serial.println(msg.buf[0], BIN);
      if (STATE_BMS_SIGNAL && STATE_IMD_SIGNAL && !STATE_HAS_LATCHED && !STATE_IS_LATCHING && msg.buf[0] & 0x10) {
        STATE_IS_LATCHING = true;
        timer_latch.reset();
        digitalWrite(SSR_LATCH, HIGH);
        digitalWrite(SSR_SOFTWARE_SHUTOFF, HIGH);
      }
    }
  }

  /*
   * Update stored states of IMD and BMS signals
   */
  STATE_IMD_SIGNAL = analogRead(SENSE_IMD);
  STATE_BMS_SIGNAL = analogRead(SENSE_BMS);

  if (!STATE_IMD_BMS_START_MET && STATE_IMD_SIGNAL > 50 && STATE_BMS_SIGNAL > 50) {
    STATE_IMD_BMS_START_MET = true;
  }

  if (STATE_IMD_BMS_START_MET && !STATE_IMD_FAULTING && STATE_IMD_SIGNAL <= 50) {
    STATE_IMD_FAULTING = true;
    timer_imd_faulting.reset();
  }

  if (STATE_IMD_BMS_START_MET && !STATE_BMS_FAULTING && STATE_BMS_SIGNAL <= 50) {
    STATE_BMS_FAULTING = true;
    timer_bms_faulting.reset();
  }

  if (STATE_IMD_FAULTING && timer_imd_faulting.check() && STATE_IMD_SIGNAL <= 50) {
    STATE_IMD_FAULT = true;
  }

  if (STATE_BMS_FAULTING && timer_bms_faulting.check() && STATE_BMS_SIGNAL <= 50) {
    STATE_BMS_FAULT = true;
  }

  /*
   * Control LEDs
   */
  if (timer_send_command.check()) {
    if (!STATE_IMD_BMS_START_MET || STATE_IMD_FAULT || STATE_BMS_FAULT) {
      msg.id = DISABLE_LED_START;
      msg.len = 1;
      msg.buf[0] = 0;
      CAN.write(msg);
    } else if (!STATE_HAS_LATCHED && !STATE_IS_LATCHING) {
      msg.id = BLINK_LED_START;
      msg.len = 1;
      msg.buf[0] = 0;
      CAN.write(msg);
    } else {
      msg.id = ENABLE_LED_START;
      msg.len = 1;
      msg.buf[0] = 0;
      CAN.write(msg);
    }
    
    if (STATE_IMD_FAULT) {
      msg.id = ENABLE_LED_IMD;
      msg.len = 1;
      msg.buf[0] = 0;
      CAN.write(msg);
    }

    if (STATE_BMS_FAULT) {
      msg.id = ENABLE_LED_BMS;
      msg.len = 1;
      msg.buf[0] = 0;
      CAN.write(msg);
    }
  }

  /*
   * Unlatch SSR
   */
  if (STATE_IS_LATCHING && timer_latch.check()) {
    STATE_IS_LATCHING = false;
    STATE_HAS_LATCHED = true;
    digitalWrite(SSR_LATCH, LOW);
  }
}
