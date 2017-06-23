/*
 * HyTech 2017 Vehicle Front Control Unit
 * Init 2017-05-13
 * Interface with dashboard lights, buttons, and buzzer.
 * Read pedal sensor values and communicate with motor controller.
 * Configured for Pedal Box Board rev3
 */
#include <FlexCAN.h>
#include "HyTech17.h"
#include <Metro.h>

/*
 * Pin definitions
 */
#define BSPD_FAULT A7
#define BTN_START A5
#define LED_START 5
#define LED_BMS 6
#define LED_BSPD 13
#define LED_IMD 7
#define PEDAL_BRAKE A2 //analog port of brake sensor
#define PEDAL_THROTTLE_1 A0 //first throttle sensor port
#define PEDAL_THROTTLE_2 A1 //second throttle sensor port
#define READY_SOUND 2
#define SOFTWARE_SHUTDOWN_RELAY 12

/*
 * Constant definitions
 */
// TODO some of these values need to be calibrated once hardware is installed
#define BRAKE_ACTIVE 282
#define MIN_THROTTLE_1 463 // compare pedal travel
#define MAX_THROTTLE_1 246
#define MIN_THROTTLE_2 93
#define MAX_THROTTLE_2 306
#define MIN_BRAKE 242
#define MAX_BRAKE 306
#define MAX_TORQUE 60 // Torque in Nm * 10
#define MIN_HV_VOLTAGE 950 // Used to check if Accumulator is energized

/*
 * Timers
 */
Metro timer_btn_start = Metro(10);
Metro timer_debug = Metro(500);
Metro timer_debug_faults = Metro(500);
Metro timer_debug_rear_state = Metro(500);
Metro timer_debug_torque = Metro(500);
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable
Metro timer_led_start_blink_fast = Metro(150);
Metro timer_led_start_blink_slow = Metro(400);
Metro timer_motor_controller_send = Metro(50);
Metro timer_ready_sound = Metro(2000); // Time to play RTD sound
Metro timer_can_update = Metro(100);

/*
 * Global variables
 */
bool brake_pedal_active = false; // True if brake is considered pressed
bool btn_start_debouncing = false;
uint8_t btn_start_id = 0; // Increments to differentiate separate button presses
uint8_t btn_start_new = 0;
bool btn_start_pressed = false;
bool debug = true;
bool fsae_brake_pedal_implausibility = false; // FSAE EV2.5
bool fsae_throttle_pedal_implausibility = false;
bool led_start_active = false;
uint8_t led_start_type = 0; // 0 for off, 1 for steady, 2 for fast blink, 3 for slow blink
uint8_t state;
double temperature; // Temperature of onboard thermistor
uint16_t value_pedal_brake = 0;
uint16_t value_pedal_throttle_1 = 0;
uint16_t value_pedal_throttle_2 = 0;

FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
  pinMode(BSPD_FAULT, INPUT);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(LED_BMS, OUTPUT);
  pinMode(LED_BSPD, OUTPUT);
  pinMode(LED_IMD, OUTPUT);
  pinMode(LED_START, OUTPUT);
  
  pinMode(PEDAL_BRAKE, INPUT);
  pinMode(PEDAL_THROTTLE_1, INPUT);
  pinMode(PEDAL_THROTTLE_2, INPUT);
  pinMode(READY_SOUND, OUTPUT);
  pinMode(SOFTWARE_SHUTDOWN_RELAY, OUTPUT);
  pinMode(13, OUTPUT);

  Serial.begin(115200); // init serial for PC communication
  CAN.begin();
  delay(100);
  Serial.println("CAN system and serial communication initialized");

  set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
  digitalWrite(SOFTWARE_SHUTDOWN_RELAY, HIGH);
  digitalWrite(13, HIGH); // Used to indicate power
}

void loop() {
  while (CAN.read(msg)) {
    // Handle PCU (power board) status messages
    if (msg.id == ID_PCU_STATUS) {
      // Load message into PCU_status object
      PCU_status pcu_status(msg.buf);
      if (pcu_status.get_bms_fault()) {
        digitalWrite(LED_BMS, HIGH);
        Serial.println("BMS Fault detected");
      }
      if (pcu_status.get_imd_fault()) {
        digitalWrite(LED_IMD, HIGH);
        Serial.println("IMD Fault detected");
      }
      if (debug && timer_debug_rear_state.check()) {
        Serial.print("PCU State: ");
        Serial.println(pcu_status.get_state());
      }

      // Set internal state based on PCU state
      // If initializing, start light off
      // If waiting for driver press, flash start light slow
      switch (pcu_status.get_state()) {
        case PCU_STATE_WAITING_BMS_IMD:
        set_start_led(0);
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_WAITING_DRIVER:
        set_start_led(3); // Slow blink
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_LATCHING:
        set_start_led(0);
        set_state(TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED);
        break;

        case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
        if (state < TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
          set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        }
        break;

        case PCU_STATE_FATAL_FAULT:
        set_start_led(0);
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        break;
      }
    }

    if (msg.id == ID_MC_INTERNAL_STATES) {
      MC_internal_states mc_internal_states = MC_internal_states(msg.buf);
      if (mc_internal_states.get_inverter_enable_state() && state == TCU_STATE_ENABLING_INVERTER) {
        set_state(TCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
      }
    }

    if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
      MC_voltage_information mc_voltage_information = MC_voltage_information(msg.buf);
      
      if (mc_voltage_information.get_dc_bus_voltage() >= MIN_HV_VOLTAGE && state == TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
        set_state(TCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
      }
      if (mc_voltage_information.get_dc_bus_voltage() < MIN_HV_VOLTAGE && state > TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
      }
    }

    if (msg.id == ID_MC_FAULT_CODES && debug && timer_debug_faults.check()) {
      MC_fault_codes mc_fault_codes = MC_fault_codes(msg.buf);
      Serial.print("Fault 1: ");
      Serial.println(mc_fault_codes.get_post_fault_lo());
      Serial.print("Fault 2: ");
      Serial.println(mc_fault_codes.get_post_fault_hi());
      Serial.print("Fault 3: ");
      Serial.println(mc_fault_codes.get_run_fault_lo());
      Serial.print("Fault 4: ");
      Serial.println(mc_fault_codes.get_run_fault_hi());
    }
  }

  /*
   * Send state over CAN
   */
  if (timer_can_update.check()) {
    // Send Throttle Control Unit message
    TCU_status tcu_status = TCU_status(fsae_throttle_pedal_implausibility,
      0, fsae_brake_pedal_implausibility, brake_pedal_active, state);
    tcu_status.write(msg.buf);
    msg.id = ID_TCU_STATUS;
    msg.len = sizeof(CAN_message_tcu_status_t);
    CAN.write(msg);

    // Send second Throttle Control Unit message
    read_values(); // Calculate new values to send
    TCU_readings tcu_readings = TCU_readings(value_pedal_throttle_1, value_pedal_throttle_2, value_pedal_brake, temperature);
    tcu_readings.write(msg.buf);
    msg.id = ID_TCU_READINGS;
    msg.len = sizeof(CAN_message_tcu_readings_t);
    CAN.write(msg);

    // Send Dashboard Control Unit message
    DCU_status dcu_status = DCU_status(btn_start_id, 0, 0, 0); // Nothing currently relies on the other data, so sending 0s for now
    dcu_status.write(msg.buf);
    msg.id = ID_DCU_STATUS;
    msg.len = sizeof(CAN_message_dcu_status_t);
    CAN.write(msg);
  }

  /*
   * State machine
   */
  switch (state) {
    case TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED:
    break;

    case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
    if (btn_start_new == btn_start_id) { // Start button has been pressed
      if (brake_pedal_active) { // Required to hold brake pedal to activate motor controller
        set_state(TCU_STATE_ENABLING_INVERTER);
      } else {
        btn_start_new = btn_start_id + 1;
      }
    }
    break;

    case TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
    break;

    case TCU_STATE_ENABLING_INVERTER:
      if (timer_inverter_enable.check()) { // Inverter enable timeout
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
      }
    break;

    case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
    if (timer_ready_sound.check()) { // RTDS has sounded
      set_state(TCU_STATE_READY_TO_DRIVE);
    }
    break;

    case TCU_STATE_READY_TO_DRIVE:
    if (timer_motor_controller_send.check()) {
      MC_command_message mc_command_message = MC_command_message(0, 0, 0, 1, 0, 0);
      read_values(); // Read new sensor values

      // Check for throttle implausibility FSAE EV2.3.10
      fsae_throttle_pedal_implausibility = false;
      /*if (value_pedal_throttle_1 < MIN_THROTTLE_1 || value_pedal_throttle_1 > MAX_THROTTLE_1) {
        fsae_throttle_pedal_implausibility = true;
      }
      if (value_pedal_throttle_2 < MIN_THROTTLE_2 || value_pedal_throttle_2 > MAX_THROTTLE_2) {
        fsae_throttle_pedal_implausibility = true;
      }*/

      // Calculate torque value
      int calculated_torque = 0;
      if (!fsae_throttle_pedal_implausibility) {
        int torque1 = map(value_pedal_throttle_1, MIN_THROTTLE_1, MAX_THROTTLE_1, 0, MAX_TORQUE);
        int torque2 = map(value_pedal_throttle_2, MIN_THROTTLE_2, MAX_THROTTLE_2, 0, MAX_TORQUE);
        /*if (abs(torque1 - torque2) * 100 / MAX_TORQUE > 10) { // Second throttle implausibility check FSAE EV2.3.6
          fsae_throttle_pedal_implausibility = true;
        } else {*/
          calculated_torque = min(torque1, torque2);
          Serial.print("Requested torque: ");
          Serial.println(calculated_torque);
          if (calculated_torque > 60) {
            calculated_torque = 30;
          }
          if (calculated_torque < 0) {
            calculated_torque = 0;
          }
        /*}*/
      }

      // FSAE EV2.5 APPS / Brake Pedal Plausibility Check
      if (fsae_brake_pedal_implausibility && !brake_pedal_active && calculated_torque <= (MAX_TORQUE / 4)) {
        fsae_brake_pedal_implausibility = false; // Clear implausibility
      }
      if (brake_pedal_active && calculated_torque > (MAX_TORQUE / 4)) {
        fsae_brake_pedal_implausibility = true;
      }

      if (fsae_brake_pedal_implausibility || fsae_throttle_pedal_implausibility) {
        // Implausibility exists, command 0 torque
        mc_command_message.set_torque_command(0);
        if (debug && timer_debug_torque.check()) {
          Serial.print("IMPLAUSIBILITY -- Throttle: ");
          Serial.print(fsae_throttle_pedal_implausibility);
          Serial.print(" -- Brake: ");
          Serial.println(fsae_brake_pedal_implausibility);
        }
      } else {
        mc_command_message.set_torque_command(calculated_torque);
        if (debug && timer_debug_torque.check()) {
          Serial.print("Calculated Torque: ");
          Serial.println(calculated_torque);
        }
      }

      mc_command_message.write(msg.buf);
      msg.id = ID_MC_COMMAND_MESSAGE;
      msg.len = 8;
      CAN.write(msg);
    }
    break;
  }

  /*
   * Send a message to the Motor Controller over CAN when vehicle is not ready to drive
   */
  if (state < TCU_STATE_READY_TO_DRIVE && timer_motor_controller_send.check()) {
    MC_command_message mc_command_message = MC_command_message(0, 0, 0, 0, 0, 0);
    if (state >= TCU_STATE_ENABLING_INVERTER) {
      mc_command_message.set_inverter_enable(true);
    }
    mc_command_message.write(msg.buf);
    msg.id = ID_MC_COMMAND_MESSAGE;
    msg.len = 8;
    CAN.write(msg);
  }

  /*
   * Blink start led
   */
  if ((led_start_type == 2 && timer_led_start_blink_fast.check()) || (led_start_type == 3 && timer_led_start_blink_slow.check())) {
    if (led_start_active) {
      digitalWrite(LED_START, LOW);
    } else {
      digitalWrite(LED_START, HIGH);
    }
    led_start_active = !led_start_active;
  }

  /*
   * Handle start button press and depress
   */
  if (digitalRead(BTN_START) == btn_start_pressed && !btn_start_debouncing) { // Value is different than stored
    btn_start_debouncing = true;
    timer_btn_start.reset();
  }
  if (btn_start_debouncing && digitalRead(BTN_START) != btn_start_pressed) { // Value returns during debounce period
    btn_start_debouncing = false;
  }
  if (btn_start_debouncing && timer_btn_start.check()) { // Debounce period finishes without value returning
    btn_start_pressed = !btn_start_pressed;
    if (btn_start_pressed) {
      btn_start_id++;
      Serial.print("Start button pressed id ");
      Serial.println(btn_start_id);
    }
  }

  /*
   * Illuminate BSPD fault LED
   */
  digitalWrite(LED_BSPD, digitalRead(BSPD_FAULT));
}

/*
 * Read values of sensors
 */
void read_values() {
    value_pedal_throttle_1 = analogRead(PEDAL_THROTTLE_1);
    value_pedal_throttle_2 = analogRead(PEDAL_THROTTLE_2);
    value_pedal_brake = analogRead(PEDAL_BRAKE);
    if (value_pedal_brake >= BRAKE_ACTIVE) {
      brake_pedal_active = true;
    } else {
      brake_pedal_active = false;
    }
    if (debug && timer_debug.check()) {
      Serial.print("Throttle 1: ");
      Serial.print(value_pedal_throttle_1);
      Serial.print("     Throttle 2: ");
      Serial.print(value_pedal_throttle_2);
      Serial.print("     Brake: ");
      Serial.print(value_pedal_brake);
      Serial.print("     Brake Pedal Active: ");
      Serial.println(brake_pedal_active);
      Serial.print("State: ");
      Serial.println(state);
    }
    // TODO calculate temperature
}

/*
 * Set the Start LED
 */
void set_start_led(uint8_t type) {
  if (led_start_type != type) {
    led_start_type = type;

    if (type == 0) {
      digitalWrite(LED_START, LOW);
      led_start_active = false;
      Serial.println("Setting Start LED off");
      return;
    }

    digitalWrite(LED_START, HIGH);
    led_start_active = true;

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
  if (state == new_state) {
    return;
  }
  state = new_state;
  if (new_state == TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
    set_start_led(0);
  }
  if (new_state == TCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
    set_start_led(2);
    btn_start_new = btn_start_id + 1;
  }
  if (new_state == TCU_STATE_ENABLING_INVERTER) {
    set_start_led(1);
    Serial.println("Enabling inverter");
    MC_command_message mc_command_message = MC_command_message(0, 0, 0, 1, 0, 0);
    msg.id = 0xC0;
    msg.len = 8;
    for(int i = 0; i < 10; i++) {
      mc_command_message.write(msg.buf); // many enable commands
      CAN.write(msg);
    }
    mc_command_message.set_inverter_enable(false);
    mc_command_message.write(msg.buf); // disable command
    CAN.write(msg);
    for(int i = 0; i < 10; i++) {
      mc_command_message.set_inverter_enable(true);
      mc_command_message.write(msg.buf); // many more enable commands
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
    Serial.println("Ready to drive");
  }
}
