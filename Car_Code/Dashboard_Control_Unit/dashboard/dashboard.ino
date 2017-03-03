#include <FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>

/******* PIN definitions ***********/
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

/*****Dashboard States and Fault Flags********/
bool imd_fault;
bool bms_fault;
bool bspd_fault;
bool cool_fault;
bool comm_fault;
bool motor_fault;
bool pedal_fault;
bool general_fault;

Metro timer_btn_start = Metro(10);
Metro timer_led_start_blink_fast = Metro(250);
Metro timer_led_start_blink_slow = Metro(500);
Metro timer_inverter_enable = Metro(2000);  // Timeout failed inverter enable
Metro timer_ready_sound = Metro(2000);      // Time to play RTD sound

unsigned long lastDebounceTOGGLE = 0;  // the last time the output pin was toggled
unsigned long lastDebounceBOOST = 0;  // the last time the output pin was toggled
unsigned long lastDebounceSTART = 0;  // the last time the output pin was toggled
uint8_t btn_start_new = 0;
bool btn_start_pressed = false;
bool led_start_active = false;
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
uint8_t led_start_type = 0;

/*************** BUTTON TYPES ****************
 *  Start Button
 *  Toggle Button
 *  Select Button
 *  Boost Button
*/
int count;

/**
 * CAN Variables
 */
FlexCAN can(500000);
FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
    // put your setup code here, to run once:
    imd_fault = false;
    bms_fault = false;
    bspd_fault = false;
    cool_fault = false;
    comm_fault = false;
    motor_fault = false;
    pedal_fault = false;
    general_fault = false;
    state = DCU_STATE_INITIAL_STARTUP;
    pinMode(LED_BMS, OUTPUT);
    pinMode(LED_IMD, OUTPUT);
    Serial.begin(115200);
    can.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  while (CAN.read(msg)) {
    // Handle PCU (power board) status messages
    if (msg.id == ID_PCU_STATUS) {
      // Load message into PCU_status object
      PCU_status pcu_status(msg.buf);

      Serial.print("PCU State: ");
      Serial.println(pcu_status.get_state());

      // Handle PCU fault states
      if (pcu_status.get_bms_fault()) {
        bms_fault = true;
        digitalWrite(LED_BMS, HIGH);
      }
      if (pcu_status.get_imd_fault()) {
        imd_fault = true;
        digitalWrite(LED_IMD, HIGH);
      }
      // Set Dashboard internal state based on PCU state
      // If not ready to power up, or ready to drive, start light off
      // If ready and waiting for 1st press, flash start light slow
      // If waiting for 2nd press, flash start light fast
      switch (pcu_status.get_state()) {
        case PCU_STATE_WAITING_BMS_IMD:
          set_start_led(0);
          set_state(DCU_STATE_WAITING_TRACTIVE_SYSTEM);
          break;
        case PCU_STATE_WAITING_DRIVER:
          set_start_led(3); // slow blink
          set_state(DCU_STATE_WAITING_TRACTIVE_SYSTEM);
          break;
        case PCU_STATE_LATCHING:
          set_start_led(0);
          set_state(DCU_STATE_PRESSED_TRACTIVE_SYSTEM);
          break;
        case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
          set_state(DCU_STATE_WAITING_MC_ENABLE);
          break;
        case PCU_STATE_FATAL_FAULT:
          set_state(DCU_STATE_FATAL_FAULT);
          break;
      }
    }

    // Handle TCU broadcast state messages
    if (msg.id == ID_TCU_STATUS) {
        TCU_status tcu_status(msg.buf);
        Serial.print("TCU State: ");
        Serial.println(tcu_status.get_state());
        switch (tcu_status.get_state()) {
            case TCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
                set_state(DCU_STATE_WAITING_MC_ENABLE);
                break;
            case TCU_STATE_WAITING_READY_TO_DRIVE_SOUND:
                set_state(DCU_STATE_PLAYING_RTD);
                break;
            case TCU_STATE_READY_TO_DRIVE:
                set_state(DCU_STATE_READY_TO_DRIVE);
                break;
        }
    }

    // Handle motor controller state messages
    // if (msg.id == ID_MC_INTERNAL_STATES) {
    //     MC_internal_states mc_internal_states = MC_internal_states(msg.buf);
    //     // if start button has been pressed and inverter is enabled, play RTD sound
    //     if (mc_internal_states.get_inverter_enable_state && state == DCU_STATE_PRESSED_MC_ENABLE) {
    //         set_state(DCU_STATE_PLAYING_RTD);
    //     }
    // }

    // TODO: Could be replaced by TCU status messages?

    // // Handle motor controller voltage messages
    // if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
    //     MC_voltage_information mc_voltage_information = MC_voltage_information(msg.buf);
    //
    // }
  }

  // TODO: more state machine stuff possibly?
  /*
   * State machine
   */
  switch (state) {
      case DCU_STATE_WAITING_TRACTIVE_SYSTEM:
        if (btn_start_new == lastDebounceSTART) {
            set_state(DCU_STATE_PRESSED_TRACTIVE_SYSTEM);
        }
        break;
      case DCU_STATE_WAITING_MC_ENABLE:
        if (btn_start_new == lastDebounceSTART) {
            set_state(DCU_STATE_PRESSED_MC_ENABLE);
        }
        break;
      case DCU_STATE_PRESSED_MC_ENABLE:
        if (timer_inverter_enable.check()) {    // inverter did not enable
            set_state(DCU_STATE_TS_INACTIVE);
        }
        break;
      case DCU_STATE_PLAYING_RTD:
        if (timer_ready_sound.check()) {        // RTD sound is finished
            set_state(DCU_STATE_READY_TO_DRIVE);
        }
        break;
  }

  // TODO: Implement broadcast of state messages

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

  // TODO: other buttons
  pollForButtonPress(); // fix this
}

void pollForButtonPress {
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
      lastDebounceSTART++;
      Serial.print("Start button pressed id ");
      Serial.println(lastDebounceSTART);
    }
  }
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

void set_state(uint8_t new_state) {
    if (state == new_state)
        return;
    state = new_state;
    if (new_state == DCU_STATE_WAITING_MC_ENABLE || new_state == DCU_STATE_WAITING_TRACTIVE_SYSTEM) {
        btn_start_new = lastDebounceSTART + 1;
    }
    if (new_state == DCU_STATE_PRESSED_MC_ENABLE || new_state == DCU_STATE_PRESSED_TRACTIVE_SYSTEM) {
        // Send CAN message indicating button press
    }
    if (new_state == DCU_STATE_PLAYING_RTD) {
        timer_ready_sound.reset();
        digitalWrite(READY_SOUND, HIGH);
        Serial.println("Playing RTD sound");
    }
    if (new_state == DCU_STATE_READY_TO_DRIVE) {
        digitalWrite(READY_SOUND, LOW);
        Serial.println("RTD sound finished");
    }
}

void toggleButtonInterrupt {

}

void startButtonInterrupt {
}
