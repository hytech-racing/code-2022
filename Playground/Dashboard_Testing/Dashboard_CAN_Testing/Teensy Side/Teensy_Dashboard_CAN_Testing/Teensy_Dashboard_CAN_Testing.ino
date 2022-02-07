#include <HyTech_FlexCAN.h>
#include <Metro.h>
#include <HyTech_CAN.h>

FlexCAN CAN(500000);
CAN_message_t msg;
Metro timer_can = Metro(1500);
Metro timer_light = Metro(3);

// CAN Messages
Dashboard_status dashboard_status;
MC_fault_codes mc_fault_codes;
MCU_status mcu_status;

// Error LED stays on for one second on dash after receiving error code
//mcu_status 8 bytes

// Test each can error code (including inertia read)
#define ERROR_TEST 1
int error = 0;
// Test inertia switch CAN message flag
#define INERTIA_TEST
// Test non error codes
#define NON_ERROR_TEST 0
// Test priority on 7-seg
#define PRIORITY_TEST 0
// Test button inputs
#define BUTTON_TEST 0

inline void send_msg(int id, int len);
inline void read_msg();
inline void msg_print();

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
  if (ERROR_TEST) {
    if (timer_can.check()) {
      // Error test performs each error test sequentially (mc err, BMS, IMD)
      // An error signal is sent via mc_fault_code for mc err
      // An error signal is sent via mcu_status for BMS and IMD
      // After signal is sent, a slight delay is added and the dash_status is then read
      // Error signal and received dash status are printed to console
      // Specific LED err flag also printed to console
      
      switch(error) {
        // mc err send
        case 0: 
          mc_fault_codes.set_post_fault_lo(0x1);
          memcpy(msg.buf, &mc_fault_codes, sizeof(uint64_t));
          send_msg(ID_MC_FAULT_CODES, 8);
          delay(100);
          read_msg();
          Serial.print(dashboard_status.get_mc_error_led());
          break;
        // BMS/AMS err send
        case 1:
          mcu_status.set_bms_ok_high(false);
          memcpy(msg.buf, &mcu_status, 7);
          send_msg(ID_MCU_STATUS, 7);
          delay(100);
          read_msg();
          Serial.println(dashboard_status.get_ams_led());
          break;
        // IMD err send
        case 2:
          mcu_status.set_imd_ok_high(false);
          memcpy(msg.buf, &mcu_status, 7);
          send_msg(ID_MCU_STATUS, 7);
          delay(100);
          read_msg();
          Serial.println(dashboard_status.get_imd_led());
          break;
        default:
          break;
      }
    }
  } else if (INERTIA_TEST) {
    
  } else if (NON_ERROR_TEST) {
    
  } else if (PRIORITY_TEST) {
    
  } else if (BUTTON_TEST) {
    
  }
  //     if (timer_can.check()) { // Send a message on CAN
  //         msg.id = 0x1;
  //         msg.len = sizeof(uint32_t);

  //         memcpy(msg.buf, &t, sizeof(uint32_t));
  //         CAN.write(msg);
  //         Serial.print("Sent 0x");
  //         Serial.print(msg.id, HEX);
  //         Serial.print(": ");
  //         for (unsigned int i = 0; i < msg.len; i++) {
  //             Serial.print(msg.buf[i]);
  //             Serial.print(" ");
  //         }
  //         Serial.println();
  //         digitalWrite(22, HIGH);
  //     }

  //     if (timer_light.check()) { // Turn off LED
  //         digitalWrite(2, LOW);
  //     }
}

inline void send_msg(int id, int len) {
  msg.id = id;
  msg.len = len;
  msg_print();
  CAN.write(msg);
}

inline void read_msg() {
  //does this need an availability check?
  //
  while(!CAN.available()) {
    delay(10);
  }
  CAN.read(msg);
  dashboard_status.load(msg.buf);
  msg_print();
}

inline void msg_print() {
  Serial.print("Sent 0x");
  Serial.print(msg.id, HEX);
  Serial.print(": ");
  for (unsigned int i = 0; i < msg.len; i++) {
      Serial.print(msg.buf[i]);
      Serial.print(" ");
  }
  Serial.println();
}
