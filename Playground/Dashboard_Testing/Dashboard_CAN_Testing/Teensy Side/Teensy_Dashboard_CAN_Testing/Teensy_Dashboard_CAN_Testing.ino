#include <HyTech_FlexCAN.h>
#include <Metro.h>
#include <HyTech_CAN.h>

FlexCAN CAN(500000);
CAN_message_t msg;
Metro timer_can = Metro(1500);
Metro timer_can_inertia = Metro(200);

// CAN Messages
Dashboard_status dashboard_status;
MC_fault_codes mc_fault_codes;
MCU_status mcu_status;
//buffer to 0 out mcu_status
uint8_t buf[8] = {0};

// Error LED stays on for one second on dash after receiving error code
//mcu_status 8 bytes

// Test each can error code (including inertia read)
#define ERROR_TEST 0
int error = 0;
// Test inertia switch CAN message flag
#define INERTIA_TEST 0
int status = 0;
// Test non error codes
#define NON_ERROR_TEST 1
// Test buzzer control from CAN
#define BUZZER_TEST 0
// Test button inputs
#define BUTTON_TEST 0

inline void mc_fault_test();
inline void mcu_status_test();
inline void read_msg();
inline void msg_print(bool Sent);

void setup() {
  Serial.begin(115200); // Initialize serial for PC communication
  CAN.begin();
  delay(200);
  Serial.println("CAN transceiver initialized");
  Serial.println("CAN TEST SENDER/RECEIVER");
  pinMode(22, OUTPUT);
  // CAN0_MCR &= 0xFFFDFFFF; // Enable self-reception
}

void loop() {
  delay(1000);
  // Test for different CAN transmitted error signals
  // Output relevant flag, read dash status, print dash status and relevant flag to console, pause, continue to next flag
  if (ERROR_TEST) {
    switch(error) {
      // Mc err send
      case 0:
          mc_fault_codes.set_post_fault_lo(0x1);
          mc_fault_test();
          Serial.println(dashboard_status.get_mc_error_led());
          error++;
          mc_fault_codes.load(buf);
          break;
      // BMS/AMS err send
      case 1:
        mcu_status.set_bms_ok_high(false);
        mcu_status.set_imd_ok_high(true);
        mcu_status_test();
        Serial.println(dashboard_status.get_ams_led());
        error++;
        //Serial.println("Here");
        mcu_status.load(buf);

        break;
      // IMD err send
      case 2:
        mcu_status.set_bms_ok_high(true);
        mcu_status.set_imd_ok_high(false);
        mcu_status_test();
        Serial.println(dashboard_status.get_imd_led());
        error++;
        mcu_status.load(buf);
        break;
      default:
        break;
    }
  // Test for seeing Inertia switch trip in CAN message from dash
  // Wait for available CAN message, print dash status and inertia LED flag to console
  } else if (INERTIA_TEST) {
    if (timer_can_inertia.check()) {
      while (CAN.available()) {
        CAN.read(msg);
        dashboard_status.load(msg.buf);
        Serial.println(dashboard_status.get_inertia_led());
      }
    }
  // Test for different START and MODE LED settings
  // Output relevant flag, read dash status, print dash status and relevant flag to console, pause, continue to next flag
  } else if (NON_ERROR_TEST) {
    //Serial.println("NON_ERROR_TEST");
    mcu_status.set_bms_ok_high(true);
    mcu_status.set_imd_ok_high(true);
    if (timer_can_inertia.check()) {
      switch(status) {
        // Start LED - startup
        case 0:
          mcu_status.set_state(MCU_STATE::STARTUP);
          mcu_status_test();
          Serial.println(dashboard_status.get_start_led());
          status++;
          mcu_status.load(buf);
          break;
        // Start LED - Tractive Not Active
        case 1:
          mcu_status.set_state(MCU_STATE::TRACTIVE_SYSTEM_NOT_ACTIVE);
          mcu_status_test();
          Serial.println(dashboard_status.get_start_led());
          status++;
          mcu_status.load(buf);
          break;
        // Start LED - Tractive Active
        case 2:
          mcu_status.set_state(MCU_STATE::TRACTIVE_SYSTEM_ACTIVE);
          mcu_status_test();
          Serial.println(dashboard_status.get_start_led());
          status++;
          mcu_status.load(buf);
          break;
        // Start LED - Ready to Drive
        case 3:
          mcu_status.set_state(MCU_STATE::READY_TO_DRIVE);
          mcu_status_test();
          Serial.println(dashboard_status.get_start_led());
          status++;
          mcu_status.load(buf);
          break;
        // Mode LED - Off
        case 4:
          mcu_status.set_torque_mode(1);
          mcu_status_test();
          Serial.println(dashboard_status.get_mode_led());
          status++;
          mcu_status.load(buf);
          break;
        // Mode LED - Fast
        case 5:
          mcu_status.set_torque_mode(2);
          mcu_status_test();
          Serial.println(dashboard_status.get_mode_led());
          status++;
          mcu_status.load(buf);
          break;
        // Mode LED - On
        case 6:
          mcu_status.set_torque_mode(3);
          mcu_status_test();
          Serial.println(dashboard_status.get_mode_led());
          status++;
          mcu_status.load(buf);
          break;
        default:
          break;
      }
    }
  } else if (BUTTON_TEST) {

  } else if (BUZZER_TEST) {
    delay(5000);
    mcu_status.set_activate_buzzer(true);
    mcu_status_test();
    delay(1000);
    mcu_status.set_activate_buzzer(false);
    mcu_status_test();
  }
}

// Set mcu_status buffer before calling this method
inline void mcu_status_test() {
  // Construct and print mcu_status message
  memcpy(msg.buf, &mcu_status, 7);
  msg.id = ID_MCU_STATUS;
  msg.len = 7;
  msg_print(true);

  // Write to CAN
  CAN.write(msg);

  // Delay and then read return message
  delay(100);
  //read_msg();

  // Load return message buffer into dash
  dashboard_status.load(msg.buf);

  // Print desired status flag after returning from this method
  // Serial.println(dashboard_status.get_ams_led());
}

// Set mc_fault_codes buffer before calling this method
inline void mc_fault_test() {
  // Construct and print mc_fault_codes message
  memcpy(msg.buf, &mc_fault_codes, 8);
  msg.id = ID_MC_FAULT_CODES;
  msg.len = 8;
  msg_print(true);

  // Write to CAN
  CAN.write(msg);

  // Delay and then read return message
  delay(100);
  //read_msg();

  // Load return message buffer into dash
  dashboard_status.load(msg.buf);

  // Print desired status flag after returning from this method
  // Serial.println(dashboard_status.get_ams_led());
}

inline void read_msg() {
  // Can return message availability check
  while(!CAN.available()) {
    delay(5);
  }
  // Read in return message and print to console
  CAN.read(msg);
  msg_print(false);
}

inline void msg_print(bool Sent) {
  Serial.print(Sent ? "Sent 0x" : "Received 0x");
  Serial.print(msg.id, HEX);
  Serial.print(": ");
  for (unsigned int i = 0; i < msg.len; i++) {
      Serial.print(msg.buf[i]);
      Serial.print(" ");
  }
  Serial.println();
}
