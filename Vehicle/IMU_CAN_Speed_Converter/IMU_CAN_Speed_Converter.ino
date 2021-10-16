#include <stdint.h>
#include "FlexCAN_T4.h"

// CAN Variables
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CAN_IMU; //Pins to IMU are A8 and A9 (22 and 23), which is CAN1
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_Vehicle; //Pins to the Vehicle CanBus are D0 and D1 (0 and 1), which is CAN2
CAN_message_t msg;
unsigned char len = 0;
unsigned char buf[8];
#define BASE_ID 0x470

void swap_bytes(uint8_t *low_byte, uint8_t high_byte);

// Initialize LEDs
#define IMU_LED 5
#define Vehicle_LED 6

// Options
#define DEBUG (false)
#define ZERO (false)

void setup() {
  // Initialize both sets of CAN lines
  CAN_IMU.begin();
  CAN_IMU.setBaudRate(1000000); //IMU CAN speed is 1Mbps
  CAN_Vehicle.begin();
  CAN_Vehicle.setBaudRate(500000); //Vehicle CAN speed is 500kbps

  // Set LED pinmodes
  pinMode(IMU_LED, OUTPUT);
  pinMode(Vehicle_LED, OUTPUT);

  // Zero IMU
  #if ZERO
  CAN_message_t zero_msg;
  zero_msg.id = 0x10;
  zero_msg.len = 8;
  uint8_t zero_buf[8] = {0x69, 0x6d, 0x75, 0x7a, 0x65, 0x72, 0x6f, 0x7a};
  for (int i = 0; i < 8; i++) {
    buf[i] = zero_buf[i];
  }
  CAN_IMU.write(zero_msg);  
  delay(6000); // delay 6 seconds to perform zeroing
  #endif
  
  
  #if DEBUG
  Serial.begin(9600);
  Serial.println("CAN INIT OK!");
  #endif
}

void loop() {
  if (CAN_IMU.read(msg)) {
    digitalWrite(IMU_LED, HIGH);
    
    #if DEBUG
    
    // accelerometer
    if (msg.id == BASE_ID) {
      int16_t lat_accel = ((int16_t)(msg.buf[0]) << 8) | msg.buf[1];
      int16_t long_accel = ((int16_t)(msg.buf[2]) << 8) | msg.buf[3];
      int16_t vert_accel = ((int16_t)(msg.buf[4]) << 8) | msg.buf[5];
      
      Serial.println("-----------------------------");
      Serial.print("Lateral:\t");
      Serial.println(lat_accel / 1000. * 9.813);
      Serial.print("Longitudinal:\t");
      Serial.println(long_accel / 1000. * 9.813);
      Serial.print("Vertical:\t");
      Serial.println(vert_accel / 1000. * 9.813);
      Serial.println();
    } 

    // gyroscope
    if (msg.id == BASE_ID + 1) {
      // multiply by 360 to get degrees
      int16_t yaw = (((int16_t)(msg.buf[0]) << 8) | msg.buf[1]) * 360;
      int16_t pitch = (((int16_t)(msg.buf[2]) << 8) | msg.buf[3]) * 360;
      int16_t roll = (((int16_t)(msg.buf[4]) << 8) | msg.buf[5]) * 360;

      Serial.println("-----------------------------");
      Serial.print("Yaw:\t");
      Serial.println(yaw / 1000.);
      Serial.print("Pitch:\t");
      Serial.println(pitch / 1000.);
      Serial.print("Roll:\t");
      Serial.println(roll / 1000.);
      Serial.println();
    }
    #endif

    swap_bytes(&msg.buf[1], &msg.buf[0]);
    swap_bytes(&msg.buf[3], &msg.buf[2]);
    swap_bytes(&msg.buf[5], &msg.buf[4]);

    CAN_Vehicle.write(msg);
    digitalWrite(Vehicle_LED, HIGH);
  }

  // turn both LEDS off 
  else {
    digitalWrite(IMU_LED, LOW);
    digitalWrite(Vehicle_LED, LOW);
  }
}

void swap_bytes(uint8_t *low_byte, uint8_t *high_byte) {
  uint8_t temp = *low_byte;
  *low_byte = *high_byte;
  *high_byte = temp;
}
