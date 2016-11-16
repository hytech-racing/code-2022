/**
 * Nathan Cheek
 * 2016-11-15
 * Control Shutdown Circuit initialization.
 */
#include <FlexCAN.h>
#include <Metro.h>

/*
 * State definitions
 */
#define PCU_STATE_WAITING_BMS_IMD 1
#define PCU_STATE_WAITING_DRIVER 2
#define PCU_STATE_LATCHING 3
#define PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED 4
#define PCU_STATE_FATAL_FAULT 5
#define TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED 1
#define TCU_STATE_TRACTIVE_SYSTEM_ACTIVE 2
#define TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE 3
#define TCU_STATE_ENABLING_INVERTER 4
#define TCU_STATE_WAITING_READY_TO_DRIVE_SOUND 5
#define TCU_STATE_READY_TO_DRIVE 6

/*
 * Pin definitions
 */
#define SSR_BRAKE_LIGHT 13
#define SSR_SOFTWARE_SHUTOFF 11
#define SSR_LATCH 10
#define SENSE_BMS A0
#define SENSE_IMD A1
#define SENSE_SHUTDOWN_OUT A2
#define SENSE_BRAKE A3
#define SENSE_TEMP A4

/*
 * CAN ID definitions
 */
#define PCU_STATUS 0xCF
#define TCU_STATUS 0xD0
#define ENABLE_LED_START 0xD1
#define DISABLE_LED_START 0xD2
#define ENABLE_LED_BMS 0xD3
#define DISABLE_LED_BMS 0xD4
#define ENABLE_LED_IMD 0xD5
#define DISABLE_LED_IMD 0xD6
#define BLINK_LED_START 0xD7

/*
 * Timers
 */
Metro timer_latch = Metro(1000);
Metro timer_state_send = Metro(100);
Metro timer_bms_faulting = Metro(100); // At startup the BMS DISCHARGE_OK line drops shortly
Metro timer_imd_faulting = Metro(100); // At startup the IMD OKHS line drops shortly

/*
 * Global variables
 */
boolean BMS_FAULT = false;
boolean IMD_FAULT = false;
boolean BMS_FAULTING = false;
boolean IMD_FAULTING = false;
uint8_t STATE = PCU_STATE_WAITING_BMS_IMD;

FlexCAN CAN(500000);
static CAN_message_t msg;

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
    if (msg.id > 0xCA) { // Ignore Motor Controller messages
      Serial.print(msg.id, HEX);
      Serial.print(": ");
      for (unsigned int i = 0; i < msg.len; i++) {
        Serial.print(msg.buf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }

    if (STATE == PCU_STATE_WAITING_DRIVER && msg.id == TCU_STATUS && msg.buf[0] & 0x10) {
      STATE = PCU_STATE_LATCHING;
      timer_latch.reset();
      digitalWrite(SSR_LATCH, HIGH);
      digitalWrite(SSR_SOFTWARE_SHUTOFF, HIGH);
      Serial.println("Latching");
    }
  }

  /*
   * Send state over CAN
   */
  if (timer_state_send.check()) {
    msg.id = PCU_STATUS;
    msg.len = 8;
    msg.buf[0] = STATE << 4 | BMS_FAULT << 3 | IMD_FAULT << 2;
    // TODO add more information to status message
    CAN.write(msg);
  }

  /*
   * Wait till IMD and BMS signals go high at startup
   */
  if (STATE == PCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_IMD) > 50 && analogRead(SENSE_BMS) > 50) {
    STATE = PCU_STATE_WAITING_DRIVER;
  }
  
  /*
   * Start BMS fault timer if signal drops momentarily
   */
  if (STATE != PCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_BMS) <= 50) {
    BMS_FAULTING = true;
    timer_bms_faulting.reset();
  }

  /*
   * Declare BMS fault if signal still dropped
   */
  if (BMS_FAULTING && timer_imd_faulting.check() && analogRead(SENSE_BMS) <= 50) {
    BMS_FAULT = true;
    STATE = PCU_STATE_FATAL_FAULT;
    digitalWrite(SSR_SOFTWARE_SHUTOFF, LOW);
    Serial.println("BMS fault detected");
  }

  /*
   * Start IMD fault timer if signal drops momentarily
   */
  if (STATE != PCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_IMD) <= 50) {
    IMD_FAULTING = true;
    timer_imd_faulting.reset();
  }

  /*
   * Declare IMD fault if signal still dropped
   */
  if (IMD_FAULTING && timer_imd_faulting.check() && analogRead(SENSE_IMD) <= 50) {
    IMD_FAULT = true;
    STATE = PCU_STATE_FATAL_FAULT;
    digitalWrite(SSR_SOFTWARE_SHUTOFF, LOW);
    Serial.println("IMD fault detected");
  }

  /*
   * Disable latching SSR
   */
  if (STATE == PCU_STATE_LATCHING && timer_latch.check()) {
    STATE = PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED;
    digitalWrite(SSR_LATCH, LOW);
  }
}
