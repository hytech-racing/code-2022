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
uint8_t buf[8];
MCU_status mcu_status;

// Error LED stays on for one second on dash after receiving error code
//mcu_status 8 bytes

// Test each can error code (including inertia read)
#define ERROR_TEST 1
int error = 0;
// Test non error codes
#define NON_ERROR_TEST 0
// Test priority on 7-seg
#define PRIORITY_TEST 0
// Test button inputs
#define BUTTON_TEST 0

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
//    if (ERROR_TEST) {
//        if (timer_can.check()) {
//            //switch()
//            msg.id = ID_MC_FAULT_CODES;
//            msg.len = sizeof(uint64_t);
//            //run_fault_high
//            buf[0] = 0x1;
//
//            mc_fault_codes.load(buf);
//            
//        }
//    }
//    if (timer_can.check()) { // Send a message on CAN
//        msg.id = 0x1;
//        msg.len = sizeof(uint32_t);
//
//        memcpy(msg.buf, &t, sizeof(uint32_t));
//        CAN.write(msg);
//        Serial.print("Sent 0x");
//        Serial.print(msg.id, HEX);
//        Serial.print(": ");
//        for (unsigned int i = 0; i < msg.len; i++) {
//            Serial.print(msg.buf[i]);
//            Serial.print(" ");
//        }
//        Serial.println();
//        digitalWrite(22, HIGH);
//    }
//    
//    if (timer_light.check()) { // Turn off LED
//        digitalWrite(2, LOW);
//    }
}
