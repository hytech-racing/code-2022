/**
 * Nathan Cheek
 * 2016-11-18
 * Interface with dashboard lights, buttons, and buzzer. Read pedal sensor values.
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
#define BTN_TOGGLE 9
#define BTN_CYCLE 10
#define BTN_BOOST 11
#define BTN_START 12
#define LED_START 7
#define LED_BMS 6
#define LED_IMD 5
#define READY_SOUND 8
#define PEDAL_SIGNAL_A A3
#define PEDAL_SIGNAL_B A4
#define PEDAL_SIGNAL_C A5

/*
 * CAN ID definitions
 */
#define PCU_STATUS 0xD0
#define TCU_STATUS 0xD1

/*
 * Timers
 */
Metro timer_state_send = Metro(100);
Metro timer_motor_controller_send = Metro(50);
Metro timer_led_start_blink_fast = Metro(150);
Metro timer_led_start_blink_slow = Metro(400);
Metro timer_ready_sound = Metro(2000);
Metro timer_btn_start = Metro(10);
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable

/*
 * Global variables
 */
boolean BTN_START_DEBOUNCING = false;
uint8_t BTN_START_ID = 0; // increments to differentiate separate button presses
uint8_t BTN_START_NEW = 0;
boolean BTN_START_PRESSED = false;
boolean LED_START_ACTIVE = false;
uint8_t LED_START_TYPE = 0; // 0 for off, 1 for steady, 2 for fast blink, 3 for slow blink
uint8_t STATE = TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED;

FlexCAN CAN(500000);
static CAN_message_t msg;

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
   * Handle incoming CAN messages
   */
  while (CAN.read(msg)) {
    if (msg.id == PCU_STATUS) {
      if (msg.buf[0] >> 3 & 0x1) { // BMS Fault
        digitalWrite(LED_BMS, HIGH);
        Serial.println("BMS Fault detected");
      }
      if (msg.buf[0] >> 2 & 0x1) { // IMD Fault
        digitalWrite(LED_IMD, HIGH);
        Serial.println("IMD Fault detected");
      }
      switch (msg.buf[0] >> 4) {
        case PCU_STATE_WAITING_BMS_IMD:
        set_start_led(0);
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_WAITING_DRIVER:
        set_start_led(3);
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_LATCHING:
        set_start_led(0);
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
        if (STATE < TCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
          set_start_led(2);
          set_state(TCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
        }
        break;

        case PCU_STATE_FATAL_FAULT:
        set_start_led(0);
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        break;
      }
    }

    if (msg.id == 0xAA && msg.buf[6] == 1 && STATE == TCU_STATE_ENABLING_INVERTER) {
      set_state(TCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
    }

    // Check DC voltage and go to tractive system active or inactive (cockpit brb)
  }

  /*
   * Send state over CAN
   */
  if (timer_state_send.check()) {
    msg.id = TCU_STATUS;
    msg.len = 2;
    msg.buf[0] = STATE;
    msg.buf[1] = BTN_START_ID;
    CAN.write(msg);
  }

  /*
   * State machine
   */
  switch (STATE) {
    case TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED:
    break;

    case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
    if (BTN_START_NEW == BTN_START_ID) { // Start button has been pressed
      set_state(TCU_STATE_ENABLING_INVERTER);
    }
    break;

    case TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
    break;

    case TCU_STATE_ENABLING_INVERTER:
      if (timer_inverter_enable.check()) {
        set_state(TCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
      }
    break;

    case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
    if (timer_ready_sound.check()) {
      set_state(TCU_STATE_READY_TO_DRIVE);
    }
    break;

    case TCU_STATE_READY_TO_DRIVE:
    if (timer_motor_controller_send.check()) {
      int torque = calculate_torque();
      Serial.println(torque);
      msg.id = 0xC0;
      msg.len = 8;
      generate_MC_message(msg.buf, torque, false, true);
      CAN.write(msg);
    }
    break;
  }

  /*
   * Send a message to the Motor Controller over CAN when vehicle is not ready to drive
   */
  if (timer_motor_controller_send.check()) {
    msg.id = 0xC0;
    msg.len = 8;
    if (STATE < TCU_STATE_ENABLING_INVERTER) {
      generate_MC_message(msg.buf, 0, false, false);
    } else if (STATE < TCU_STATE_READY_TO_DRIVE) {
      generate_MC_message(msg.buf, 0, false, true);
    }
    CAN.write(msg);
  }

  /*
   * Blink start led
   */
  if ((LED_START_TYPE == 2 && timer_led_start_blink_fast.check()) || (LED_START_TYPE == 3 && timer_led_start_blink_slow.check())) {
    if (LED_START_ACTIVE) {
      digitalWrite(LED_START, LOW);
    } else {
      digitalWrite(LED_START, HIGH);
    }
    LED_START_ACTIVE = !LED_START_ACTIVE;
  }

  /*
   * Handle start button press and depress
   */
  if (digitalRead(BTN_START) == BTN_START_PRESSED && !BTN_START_DEBOUNCING) { // Value is different than stored
    BTN_START_DEBOUNCING = true;
    timer_btn_start.reset();
  }
  if (BTN_START_DEBOUNCING && digitalRead(BTN_START) != BTN_START_PRESSED) { // Value returns during debounce period
    BTN_START_DEBOUNCING = false;
  }
  if (BTN_START_DEBOUNCING && timer_btn_start.check()) { // Debounce period finishes without value returning
    BTN_START_PRESSED = !BTN_START_PRESSED;
    if (BTN_START_PRESSED) {
      BTN_START_ID++;
      Serial.print("Start button pressed id ");
      Serial.println(BTN_START_ID);
    }
  }
}

void generate_MC_message(unsigned char* message, int torque, boolean backwards, boolean enable) {
  message[0] = torque & 0xFF;
  message[1] = torque >> 8;
  message[2] = 0;
  message[3] = 0;
  message[4] = char(backwards);
  message[5] = char(enable);
  message[6] = 0;
  message[7] = 0;
}

int calculate_torque() {
  /*Serial.print("A ");
  Serial.print(analogRead(PEDAL_SIGNAL_A));
  Serial.print(" B ");
  Serial.print(analogRead(PEDAL_SIGNAL_B));
  Serial.print(" C ");
  Serial.println(analogRead(PEDAL_SIGNAL_C));*/
  int val = analogRead(PEDAL_SIGNAL_A);
  if (val < 150) {
    val = 150;
  }
  if (val > 260) {
    val = 260;
  }
  return map(val, 150, 260, 0, 200);
}

/*
 * Set the Start LED
 */
void set_start_led(uint8_t type) {
  if (LED_START_TYPE != type) {
    LED_START_TYPE = type;

    if (type == 0) {
      digitalWrite(LED_START, LOW);
      LED_START_ACTIVE = false;
      Serial.println("Setting Start LED off");
      return;
    }
    
    digitalWrite(LED_START, HIGH);
    LED_START_ACTIVE = true;

    if (type == 1) {
      Serial.println("Setting Start LED solid on");
    } else if (type == 2) {
      timer_led_start_blink_fast.reset();
      Serial.println("Setting Start LED fast blink");
    } else if (type == 3) {
      timer_led_start_blink_slow.reset();
      Serial.println("Setting Start LED slow blink");
    }
  }
}

/*
 * Handle changes in state
 */
void set_state(uint8_t new_state) {
  if (STATE == new_state) {
    return;
  }
  STATE = new_state;
  if (new_state == TCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
    BTN_START_NEW = BTN_START_ID + 1;
  }
  if (new_state == TCU_STATE_ENABLING_INVERTER) {
    set_start_led(1);
    Serial.println("Enabling inverter");
    msg.id = 0xC0;
    msg.len = 8;
    for(int i = 0; i < 10; i++) {
      generate_MC_message(msg.buf,0,false,true); // many enable commands
      CAN.write(msg);
    }
    generate_MC_message(msg.buf,0,false,false); // disable command
    CAN.write(msg);
    for(int i = 0; i < 10; i++) {
      generate_MC_message(msg.buf,0,false,true); // many more enable commands
      CAN.write(msg);
    }
    Serial.println("Sent enable command");
    timer_inverter_enable.reset();
  }
  if (new_state == TCU_STATE_WAITING_READY_TO_DRIVE_SOUND) {
    timer_ready_sound.reset();
    digitalWrite(READY_SOUND, HIGH);
    Serial.println("Inverter enabled");
    Serial.println("RTDS enabled");
  }
  if (new_state == TCU_STATE_READY_TO_DRIVE) {
    digitalWrite(READY_SOUND, LOW);
    Serial.println("RTDS deactivated");
  }
}

