/*
 * Nathan Cheek
 * 2016-11-18
 * Interface with dashboard lights, buttons, and buzzer. Read pedal sensor values.
 */
#include <FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>

/*
 * Pin definitions
 */
#define BTN_BOOST 9 // 3
#define BTN_CYCLE 10 // 2
#define BTN_START 12
#define BTN_TOGGLE 11 // 1
#define COOLING_1 13
#define COOLING_2 A0
#define COOLING_3 A1
#define LED_BMS 6
#define LED_IMD 5
#define LED_START 7
#define PEDAL_SIGNAL_A A3
#define PEDAL_SIGNAL_B A4
#define PEDAL_SIGNAL_C A5
#define READY_SOUND 8

/*
 * Timers
 */
Metro timer_btn_cycle = Metro(10);
Metro timer_btn_start = Metro(10);
Metro timer_btn_toggle = Metro(10);
Metro timer_cooling_ramp = Metro(10);
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable
Metro timer_led_start_blink_fast = Metro(150);
Metro timer_led_start_blink_slow = Metro(400);
Metro timer_motor_controller_send = Metro(50);
Metro timer_ready_sound = Metro(2000);
Metro timer_state_send = Metro(100);

/*
 * Global variables
 */
bool btn_cycle_debouncing = false;
uint8_t btn_cycle_id = 0; // increments to differentiate separate button presses
uint8_t btn_cycle_new = 0;
bool btn_cycle_pressed = false;
bool btn_start_debouncing = false;
uint8_t btn_start_id = 0; // increments to differentiate separate button presses
uint8_t btn_start_new = 0;
bool btn_start_pressed = false;
bool btn_toggle_debouncing = false;
uint8_t btn_toggle_id = 0; // increments to differentiate separate button presses
uint8_t btn_toggle_new = 0;
bool btn_toggle_pressed = false;
uint16_t button_torque = 0;
bool cooling_ramp_enable = false;
uint8_t cooling_ramp = 0;
bool led_start_active = false;
uint8_t led_start_type = 0; // 0 for off, 1 for steady, 2 for fast blink, 3 for slow blink
uint8_t state = TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED;

FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
  pinMode(BTN_BOOST, INPUT_PULLUP);
  pinMode(BTN_CYCLE, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_TOGGLE, INPUT_PULLUP);
  pinMode(COOLING_1, OUTPUT);
  pinMode(COOLING_2, OUTPUT);
  pinMode(COOLING_3, OUTPUT);
  pinMode(LED_BMS, OUTPUT);
  pinMode(LED_IMD, OUTPUT);
  pinMode(LED_START, OUTPUT);
  pinMode(READY_SOUND, OUTPUT);

  Serial.begin(115200);
  CAN.begin();
  delay(100);
  Serial.println("CAN transceiver initialized");
}

void loop() {
  /*
   * Handle incoming CAN messages
   */
  while (CAN.read(msg)) {
    if (msg.id == ID_PCU_STATUS) {
      PCU_status pcu_status(msg.buf);
      if (pcu_status.get_bms_fault()) {
        digitalWrite(LED_BMS, HIGH);
        Serial.println("BMS Fault detected");
      }
      if (pcu_status.get_imd_fault()) {
        digitalWrite(LED_IMD, HIGH);
        Serial.println("IMD Fault detected");
      }
      
      Serial.print("PCU State: ");
      Serial.println(pcu_status.get_state());
      switch (pcu_status.get_state()) {
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
        if (state < TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
          set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        }
        break;

        case PCU_STATE_FATAL_FAULT:
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
        break;
      }
    }

    if (msg.id == ID_MC_MOTOR_POSITION_INFORMATION) {
      MC_motor_position_information mc_motor_position_information = MC_motor_position_information(msg.buf);
      Serial.print("Motor angle: ");
      Serial.println(mc_motor_position_information.get_motor_angle());
      Serial.print("Motor speed: ");
      Serial.println(mc_motor_position_information.get_motor_speed());
      Serial.print("Electrical output frequency: ");
      Serial.println(mc_motor_position_information.get_electrical_output_frequency());
      Serial.print("Delta resolver filtered: ");
      Serial.println(mc_motor_position_information.get_delta_resolver_filtered());
    }

    if (msg.id == ID_MC_INTERNAL_STATES) {
      MC_internal_states mc_internal_states = MC_internal_states(msg.buf);
      if (mc_internal_states.get_inverter_enable_state() && state == TCU_STATE_ENABLING_INVERTER) {
        set_state(TCU_STATE_WAITING_READY_TO_DRIVE_SOUND);
      }
      Serial.print("Inverter ");
      if (mc_internal_states.get_inverter_enable_state()) {
        Serial.println("enabled");
      } else {
        Serial.println("disabled");
      }
      Serial.print("VSM State: ");
      Serial.println(mc_internal_states.get_vsm_state());
      Serial.print("Inverter State: ");
      Serial.println(mc_internal_states.get_inverter_state());
    }

    if (msg.id == ID_MC_TEMPERATURES_3) {
      MC_temperatures_3 mc_temperatures_3 = MC_temperatures_3(msg.buf);
      Serial.print("Motor temperature: ");
      Serial.println(mc_temperatures_3.get_motor_temperature());
    }

    if (msg.id == ID_MC_TEMPERATURES_1) {
      MC_temperatures_1 mc_temperatures_1 = MC_temperatures_1(msg.buf);
      Serial.print("Module A temperature: ");
      Serial.println(mc_temperatures_1.get_module_a_temperature());
      Serial.print("Module B temperature: ");
      Serial.println(mc_temperatures_1.get_module_b_temperature());
      Serial.print("Module C temperature: ");
      Serial.println(mc_temperatures_1.get_module_c_temperature());
      Serial.print("Gate driver board temperature: ");
      Serial.println(mc_temperatures_1.get_gate_driver_board_temperature());
    }
      
    if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
      MC_voltage_information mc_voltage_information = MC_voltage_information(msg.buf);
      if (mc_voltage_information.get_dc_bus_voltage() > 100 && state == TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
        set_state(TCU_STATE_TRACTIVE_SYSTEM_ACTIVE);
      }
      if (mc_voltage_information.get_dc_bus_voltage() <= 100 && state > TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE) {
        set_state(TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE);
      }
      /*Serial.print("DC Bus Voltage: ");
      Serial.println(mc_voltage_information.get_dc_bus_voltage());*/
    }

    if (msg.id == ID_MC_TORQUE_TIMER_INFORMATION) {
      MC_torque_timer_information mc_torque_timer_information = MC_torque_timer_information(msg.buf);
      Serial.print("Commanded torque: ");
      Serial.print(mc_torque_timer_information.get_commanded_torque());
      Serial.println(" Nm");
    }
  }
 
  /*
   * Send state over CAN
   */
  if (timer_state_send.check()) {
    TCU_status tcu_status = TCU_status(state, btn_start_id);
    tcu_status.write(msg.buf);
    msg.id = ID_TCU_STATUS;
    msg.len = sizeof(CAN_message_tcu_status_t);
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
      set_state(TCU_STATE_ENABLING_INVERTER);
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
      int torque = button_torque;
      Serial.println(torque);
      MC_command_message message = MC_command_message(torque, 0, false, true, false, 0);
      message.write(msg.buf);
      msg.id = 0xC0;
      msg.len = 8;
      //generate_MC_message(msg.buf, torque, false, true);
      CAN.write(msg);
    }
    break;
  }

  /*
   * Send a message to the Motor Controller over CAN when vehicle is not ready to drive
   */
  if (state < TCU_STATE_READY_TO_DRIVE && timer_motor_controller_send.check()) {
    msg.id = ID_MC_COMMAND_MESSAGE;
    msg.len = 8;
    if (state < TCU_STATE_ENABLING_INVERTER) {
      generate_MC_message(msg.buf, 0, false, false);
    } else if (state < TCU_STATE_READY_TO_DRIVE) {
      generate_MC_message(msg.buf, 0, false, true);
    }
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
   * Handle cycle button press and depress
   */
  if (digitalRead(BTN_CYCLE) == btn_cycle_pressed && !btn_cycle_debouncing) { // Value is different than stored
    btn_cycle_debouncing = true;
    timer_btn_cycle.reset();
  }
  if (btn_cycle_debouncing && digitalRead(BTN_CYCLE) != btn_cycle_pressed) { // Value returns during debounce period
    btn_cycle_debouncing = false;
  }
  if (btn_cycle_debouncing && timer_btn_cycle.check()) { // Debounce period finishes without value returning
    btn_cycle_pressed = !btn_cycle_pressed;
    if (btn_cycle_pressed) {
      btn_cycle_id++;
      Serial.print("Cycle button pressed id ");
      Serial.println(btn_cycle_id);
      if (button_torque < 500 && state == TCU_STATE_READY_TO_DRIVE) {
        button_torque += 10;
      }
    }
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
   * Handle toggle button press and depress
   */
  if (digitalRead(BTN_TOGGLE) == btn_toggle_pressed && !btn_toggle_debouncing) { // Value is different than stored
    btn_toggle_debouncing = true;
    timer_btn_toggle.reset();
  }
  if (btn_toggle_debouncing && digitalRead(BTN_TOGGLE) != btn_toggle_pressed) { // Value returns during debounce period
    btn_toggle_debouncing = false;
  }
  if (btn_toggle_debouncing && timer_btn_toggle.check()) { // Debounce period finishes without value returning
    btn_toggle_pressed = !btn_toggle_pressed;
    if (btn_toggle_pressed) {
      btn_toggle_id++;
      Serial.print("Toggle button pressed id ");
      Serial.println(btn_toggle_id);
      if (button_torque > 0 && state == TCU_STATE_READY_TO_DRIVE) {
        button_torque -= 20;
      }
      if (button_torque > 500) {
        button_torque = 0;
      }
    }
  }

  /*
   * Handle cooling ramp up
   */
  if (cooling_ramp_enable && timer_cooling_ramp.check()) {
    cooling_ramp++;
    digitalWrite(COOLING_1, cooling_ramp);
    digitalWrite(COOLING_2, cooling_ramp);
    digitalWrite(COOLING_3, cooling_ramp);
    if (cooling_ramp == 200) {
      cooling_ramp_enable = false;
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
    cooling_ramp = 0;
    cooling_ramp_enable = false;
    digitalWrite(COOLING_1, cooling_ramp);
    digitalWrite(COOLING_2, cooling_ramp);
    digitalWrite(COOLING_3, cooling_ramp);
  }
  if (new_state == TCU_STATE_TRACTIVE_SYSTEM_ACTIVE) {
    set_start_led(2);
    btn_start_new = btn_start_id + 1;
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
    cooling_ramp_enable = true;
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
    button_torque = 0;
  }
}


