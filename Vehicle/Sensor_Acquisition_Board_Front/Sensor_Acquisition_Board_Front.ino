/*
 * @brief: Teensy code for the Front Sensor Acquisition Board.
 *         There are 2 suspension linear pots + the Adafruit GPS + the Racegrade IMU
 * 
 * @author: Bo Han Zhu
 * @date: 1/29/2022
 */
 
/*
 * Sensor Acquisition Board Front Ports:              Teensy 4.0 Pin Number:
 * 1: Front-Left Suspension Linear Potentiometer      A0
 * 2: Front-Right Suspension Linear Potentiometer     A1
 * 3. IMU CAN TX                                      A8
 * 4. IMU CAN RX                                      A9
 * 5. GPS TX                                          D8
 * 6. GPS RX                                          D7
 */

#include <stdint.h>
#include "HyTech_CAN.h"
#include "CAN_ID.h"
#include "Metro.h"
#include "FlexCAN_T4.h"

// CAN Variables
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CAN_IMU;      //Pins to IMU are A8 and A9 (22 and 23), which is CAN1
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_Vehicle;  //Pins to the Vehicle CanBus are D0 and D1 (0 and 1), which is CAN2
CAN_message_t sab_msg; //
CAN_message_t imu_msg; // For incoming IMU CAN message
CAN_message_t imu_accel_msg; // For outgoing IMU Accel CAN message
CAN_message_t imu_gyro_msg; // For outgoing IMU Gyro CAN message
unsigned char len = 0;
unsigned char buf[8];
SAB_readings_front sab_readings_front;
//SAB_readings_gps sab_readings_gps;

// SAB Analog Readings and Filtering
#define SENSOR_1_CHANNEL A0
#define SENSOR_2_CHANNEL A1
#define ALPHA 0.75                                                                                      // Adjust Filtering Strength (Higher = More Filtering)
inline float get_sensor1_value() {return (analogRead(SENSOR_1_CHANNEL) * 0.059312 + 3.0) * 1000;}       // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor2_value() {return (analogRead(SENSOR_2_CHANNEL) * 0.059312 + 3.0) * 1000;}       // DO NOT CHANGE THIS W/O SPECIAL REASON
float filtered_sensor1_reading{};
float filtered_sensor2_reading{};

// Timers
Metro timer_SAB_front = Metro(200);
Metro timer_IMU = Metro(5);
// Metro timer_adafruit_gps = Metro(200);

void swap_bytes(uint8_t *low_byte, uint8_t high_byte);

// Initialize LEDs
#define IMU_LED 5
#define VEHICLE_LED 6

// Options
#define DEBUG (false)
#define ZERO_IMU (false)

void setup() {
  // Initialize both sets of CAN lines
  CAN_IMU.begin();
  CAN_IMU.setBaudRate(1000000); //IMU CAN speed is 1Mbps
  CAN_Vehicle.begin();
  CAN_Vehicle.setBaudRate(500000); //Vehicle CAN speed is 500kbps

  // Set LED pinmodes
  pinMode(IMU_LED, OUTPUT);
  pinMode(VEHICLE_LED, OUTPUT);

  // Zero IMU
  #if ZERO_IMU
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
  
  //Get initial analog sensor readings
  filtered_sensor1_reading = get_sensor1_value();
  filtered_sensor2_reading = get_sensor2_value();

  //Set permanent outgoing IMU CAN msg ids
  imu_accel_msg.id = 0x9E;
  imu_gyro_msg.id = 0x9F;
}

void loop() {
  // Check if IMU CAN line sends message and save it accordingly if it does
  if (CAN_IMU.read(imu_msg)) {
    if (imu_msg.id == ID_IMU_ACCELEROMETER) {
      imu_accel_msg.len = imu_msg.len;
      memcpy(&imu_accel_msg.buf, &imu_msg.buf, sizeof(imu_msg.buf));
    } else if (imu_msg.id == ID_IMU_GYROSCOPE) {
      imu_gyro_msg.len = imu_msg.len;
      memcpy(&imu_gyro_msg.buf, &imu_msg.buf, sizeof(imu_msg.buf));
    }
  }

  // Send SAB analog readings over SAB if timer checks
  if (timer_SAB_front.check()) {
    digitalWrite(VEHICLE_LED, !digitalRead(VEHICLE_LED)); // Invert LED status to simulate flashing
    
    #if DEBUG      
    Serial.println("-----------------------------");
    Serial.print("Sensor 1:\t");
    Serial.println(filtered_sensor1_reading / 1000.0);
    Serial.print("Sensor 2:\t");
    Serial.println(filtered_sensor2_reading / 1000.0);
    Serial.println();
    #endif

    
    sab_readings_front.set_sensor_1(filtered_sensor1_reading);
    sab_readings_front.set_sensor_2(filtered_sensor2_reading);
    sab_readings_front.write(sab_msg.buf);
    sab_msg.id = ID_SAB_READINGS_FRONT;
    sab_msg.len = sizeof(sab_readings_front);
    CAN_Vehicle.write(sab_msg);
  }
  
  // Send both IMU Accel and Gyro CAN messages if timer ticks
  if (timer_IMU.check()) {
    digitalWrite(VEHICLE_LED, !digitalRead(VEHICLE_LED)); // Invert LED status to simulate flashing
    digitalWrite(IMU_LED, !digitalRead(IMU_LED));         // Invert LED status to simulate flashing
    
    #if DEBUG
    // accelerometer
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

    // gyroscope
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
    #endif

    // Fix endianness
    swap_bytes(&imu_accel_msg.buf[1], &imu_accel_msg.buf[0]);
    swap_bytes(&imu_accel_msg.buf[3], &imu_accel_msg.buf[2]);
    swap_bytes(&imu_accel_msg.buf[5], &imu_accel_msg.buf[4]);
    
    swap_bytes(&imu_gyro_msg.buf[1], &imu_gyro_msg.buf[0]);
    swap_bytes(&imu_gyro_msg.buf[3], &imu_gyro_msg.buf[2]);
    swap_bytes(&imu_gyro_msg.buf[5], &imu_gyro_msg.buf[4]);

    // Write both CAN messages
    CAN_Vehicle.write(imu_accel_msg);
    CAN_Vehicle.write(imu_gyro_msg);
  }

  // Software analog filtering
  filtered_sensor1_reading = ALPHA * filtered_sensor1_reading + (1 - ALPHA) * get_sensor1_value();;
  filtered_sensor2_reading = ALPHA * filtered_sensor2_reading + (1 - ALPHA) * get_sensor2_value();;
}

void swap_bytes(uint8_t *low_byte, uint8_t *high_byte) {
  uint8_t temp = *low_byte;
  *low_byte = *high_byte;
  *high_byte = temp;
}
