/*
 * HyTech 2017 Vehicle Rear Control Unit
 * Init 2017-06-02
 * Control Shutdown Circuit initialization.
 * Configured for Power Board rev3
 */
#include <FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>

/*
 * Pin definitions
 */
#define COOL_MOSFET_1 A8
#define COOL_MOSFET_2 A7
#define COOL_MOSFET_3 A6
#define COOL_RELAY_1 A9
#define COOL_RELAY_2 2
#define SENSE_BMS A1
#define SENSE_IMD A0
#define SENSE_SHUTDOWN_OUT A2
#define SSR_BRAKE_LIGHT 12
#define SSR_INVERTER 6
#define SSR_LATCH_BMS 11
#define SSR_LATCH_IMD 8

/*
 * Constant definitions
 */
#define BMS_HIGH 100
#define BMS_LOW 50
#define IMD_HIGH 100
#define IMD_LOW 50

/*
 * Timers
 */
Metro timer_bms_faulting = Metro(1000); // At startup the BMS DISCHARGE_OK line drops shortly
Metro timer_imd_faulting = Metro(1000); // At startup the IMD OKHS line drops shortly
Metro timer_latch = Metro(1000);
Metro timer_state_send = Metro(100);
Metro timer_tcu_restart_inverter = Metro(500); // Upon restart of the TCU, power cycle the inverter

/*
 * Global variables
 */
boolean bms_fault = false;
boolean bms_faulting = false;
uint8_t btn_start_id = 0; // increments to differentiate separate button presses
uint8_t btn_start_new = 0;
boolean imd_fault = false;
boolean imd_faulting = false;
uint8_t state = PCU_STATE_WAITING_BMS_IMD;

FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
  pinMode(COOL_RELAY_1, OUTPUT);
  pinMode(COOL_RELAY_2, OUTPUT);
  pinMode(SSR_BRAKE_LIGHT, OUTPUT);
  pinMode(SSR_INVERTER, OUTPUT);
  pinMode(SSR_LATCH_BMS, OUTPUT);
  pinMode(SSR_LATCH_IMD, OUTPUT);

  Serial.begin(115200);
  CAN.begin();
  delay(100);
  Serial.println("CAN transceiver initialized");
  digitalWrite(SSR_INVERTER, HIGH);
  digitalWrite(COOL_RELAY_1, HIGH);
  digitalWrite(COOL_RELAY_2, HIGH);
}

void loop() {
  /*
   * Handle incoming CAN messages
   */
  while (CAN.read(msg)) {
    if (msg.id == ID_DCU_STATUS) {      
      DCU_status message = DCU_status(msg.buf);
      if (btn_start_id != message.get_btn_press_id()) {
        btn_start_id = message.get_btn_press_id();
        Serial.print("Start button pressed id ");
        Serial.println(btn_start_id);
      }
    }
    if (msg.id == ID_TCU_STATUS) {
      TCU_status tcu_status = TCU_status(msg.buf);
      if (tcu_status.get_brake_pedal_active())
        digitalWrite(SSR_BRAKE_LIGHT, HIGH);
      else
        digitalWrite(SSR_BRAKE_LIGHT, LOW);
    }
    if (msg.id == ID_TCU_RESTART) {
      if (millis() > 1000) { // Ignore restart messages when this microcontroller has also just booted up
        digitalWrite(SSR_INVERTER, LOW);
        timer_tcu_restart_inverter.reset();
        set_state(0);
      }
    }
    /*if (msg.id == ID_MC_COMMAND_MESSAGE) {
      MC_command_message mc_command_message = MC_command_message(msg.buf);
      Serial.print("Torque command: ");
      Serial.println(mc_command_message.get_torque_command());
      Serial.print("Angular velocity: ");
      Serial.println(mc_command_message.get_angular_velocity());
      Serial.print("Direction: ");
      Serial.println(mc_command_message.get_direction());
      Serial.print("Inverter enable: ");
      Serial.println(mc_command_message.get_inverter_enable());
      Serial.print("Discharge enable: ");
      Serial.println(mc_command_message.get_discharge_enable());
      Serial.print("Commanded torque limit: ");
      Serial.println(mc_command_message.get_commanded_torque_limit());
    }*/
  }

  /*
   * Send state over CAN
   */
  if (timer_state_send.check()) {
    PCU_status pcu_status(state, bms_fault, imd_fault, 0, 0); // Nothing external relies on OKHS or discharge_ok voltage so sending 0s for now
    pcu_status.write(msg.buf);
    msg.id = ID_PCU_STATUS;
    msg.len = sizeof(CAN_message_pcu_status_t);
    CAN.write(msg);
  }

  switch (state) {
    case 0:
    if (timer_tcu_restart_inverter.check()) {
      digitalWrite(SSR_INVERTER, HIGH);
      set_state(PCU_STATE_WAITING_BMS_IMD);
    }
    break;
        
    case PCU_STATE_WAITING_BMS_IMD:
    if (analogRead(SENSE_IMD) > IMD_HIGH && analogRead(SENSE_BMS) > BMS_HIGH) { // Wait till IMD and BMS signals go high at startup
      set_state(PCU_STATE_WAITING_DRIVER);
    }
    break;

    case PCU_STATE_WAITING_DRIVER:
    if (btn_start_new == btn_start_id) { // Start button has been pressed
      set_state(PCU_STATE_LATCHING);
    }
    break;

    case PCU_STATE_LATCHING:
    if (timer_latch.check()) { // Disable latching SSR
      set_state(PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED);
    }
    break;

    case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
    break;

    case PCU_STATE_FATAL_FAULT:
    break;
  }

  /*
   * Start BMS fault timer if signal drops momentarily
   */
  if (state != PCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_BMS) <= BMS_LOW) { // TODO imd/bms
    bms_faulting = true;
    timer_bms_faulting.reset();
  }

  /*
   * Reset BMS fault condition if signal comes back within timer period
   */
  if (bms_faulting && analogRead(SENSE_BMS) > BMS_HIGH) {
    bms_faulting = false;
  }

  /*
   * Declare BMS fault if signal still dropped
   */
  if (bms_faulting && timer_bms_faulting.check()) {
    bms_fault = true;
    set_state(PCU_STATE_FATAL_FAULT);
    Serial.println("BMS fault detected");
  }

  /*
   * Start IMD fault timer if signal drops momentarily
   */
  if (state != PCU_STATE_WAITING_BMS_IMD && analogRead(SENSE_IMD) <= IMD_LOW) {
    imd_faulting = true;
    timer_imd_faulting.reset();
  }

  /*
   * Reset IMD fault condition if signal comes back within timer period
   */
  if (imd_faulting && analogRead(SENSE_IMD) > IMD_HIGH) {
    imd_faulting = false;
  }

  /*
   * Declare IMD fault if signal still dropped
   */
  if (imd_faulting && timer_imd_faulting.check()) {
    imd_fault = true;
    set_state(PCU_STATE_FATAL_FAULT);
    Serial.println("IMD fault detected");
  }
}

/*
 * Handle changes in state
 */
void set_state(uint8_t new_state) {
  if (state == new_state) {
    return;
  }
  state = new_state;
  if (new_state == PCU_STATE_WAITING_DRIVER) {
    btn_start_new = btn_start_id + 1;
  }
  if (new_state == PCU_STATE_LATCHING) {
    timer_latch.reset();
    digitalWrite(SSR_LATCH_BMS, HIGH);
    digitalWrite(SSR_LATCH_IMD, HIGH);
    Serial.println("Latching");
  }
  if (new_state == PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED) {
    digitalWrite(SSR_LATCH_BMS, LOW);
    digitalWrite(SSR_LATCH_IMD, LOW);
    digitalWrite(COOL_RELAY_2, HIGH);
  }
}

