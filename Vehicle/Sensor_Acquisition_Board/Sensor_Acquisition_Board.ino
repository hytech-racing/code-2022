/*
 * Sensor Acquisition Board Ports:                    Teensy 3.2 Pin Number:
 * 1: Front-Left Suspension Linear Potentiometer      A0
 * 2: Front-Right Suspension Linear Potentiometer     A1
 * 3: Steering Wheel Sensor                           A2
 * 4: Ambient Air Humidity                            A3
 * 5: Back-Left Suspension Linear Potentiometer       A4
 * 6: Back-Right Suspension Linear Potentiometer      A5
 * 7: Ambient Air Temperature                         A6
 * 8: Motor Loop Cooling Fluid Temperature            A7
 */

#include "SAB_readings.h"
#include "CAN_ID.h"
#include "HyTech_FlexCAN.h"
#include "Metro.h"

// CAN Variables
FlexCAN CAN(500000);
Metro timer_front_update = Metro(100);
Metro timer_rear_update = Metro(100);
CAN_message_t msg;

// Initialize LEDs
#define CAN_LED 5
#define CAN_LED_TICKS_LIMIT 1000
int CAN_LED_ticks = 0;

// Analog Reading
#define SENSOR_1_CHANNEL A0
#define SENSOR_2_CHANNEL A1
#define SENSOR_3_CHANNEL A2
#define SENSOR_4_CHANNEL A3
#define SENSOR_5_CHANNEL A4
#define SENSOR_6_CHANNEL A5
#define SENSOR_7_CHANNEL A6
#define SENSOR_8_CHANNEL A7
#define ALPHA 0.9772                      // parameter for the software filter used on ADC pedal channels
//#define INPUT_TO_5000mV 5.9082, expression: 3.3V/(30/11)V * 3.3V/1024counts * 5V/3.3V * 1000mV/1V = 5.9082, used just for reference
// Functions scale value up by 1000, will get scaled down 1000 later in autoparse
inline float get_sensor1_value() {return (analogRead(SENSOR_1_CHANNEL) * 0.06015 + 1.60574) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor2_value() {return (analogRead(SENSOR_2_CHANNEL) * 0.06015 + 1.60574) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor3_value() {return analogRead(SENSOR_3_CHANNEL) * 1000;} //TODO: replace this with steering wheel sensor equation
inline float get_sensor4_value() {return (analogRead(SENSOR_4_CHANNEL) * 0.22988 - 41.93897) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor5_value() {return (analogRead(SENSOR_5_CHANNEL) * 0.06015 + 1.60574) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor6_value() {return (analogRead(SENSOR_6_CHANNEL) * 0.06015 + 1.60574) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor7_value() {return (analogRead(SENSOR_7_CHANNEL) * -0.43003 + 190.95588) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor8_value() {return (analogRead(SENSOR_8_CHANNEL) * 0.26259 - 61.11111) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON


// Options
#define DEBUG (true)

SAB_readings sab_readings;

/*
 * Variables to store filtered values
 */
float filtered_sensor1_reading{};
float filtered_sensor2_reading{};
float filtered_sensor3_reading{};
float filtered_sensor4_reading{};
float filtered_sensor5_reading{};
float filtered_sensor6_reading{};
float filtered_sensor7_reading{};
float filtered_sensor8_reading{};


void setup() {
  //Initiallizes CAN
  pinMode(CAN_LED, OUTPUT);
  CAN.begin();

  //Get initial readings
  filtered_sensor1_reading = get_sensor1_value();
  filtered_sensor2_reading = get_sensor2_value();
  filtered_sensor3_reading = get_sensor3_value();
  filtered_sensor4_reading = get_sensor4_value();
  filtered_sensor5_reading = get_sensor5_value();
  filtered_sensor6_reading = get_sensor6_value();
  filtered_sensor7_reading = get_sensor7_value();
  filtered_sensor8_reading = get_sensor8_value();
}

void loop() {  
  // LEDs
  if (CAN_LED_ticks == CAN_LED_TICKS_LIMIT / 2) {
    digitalWrite(CAN_LED, HIGH);
  }
  if (CAN_LED_ticks == CAN_LED_TICKS_LIMIT) {
    digitalWrite(CAN_LED, LOW);
    CAN_LED_ticks = 0;
  }
  if (timer_front_update.check()){
      CAN_LED_ticks++;
      
      sab_readings.set_sensor_1(filtered_sensor1_reading);
      sab_readings.set_sensor_2(filtered_sensor2_reading);
      sab_readings.set_sensor_3(filtered_sensor3_reading);
      sab_readings.set_sensor_4(filtered_sensor4_reading);
      sab_readings.write(msg.buf);
      msg.id = ID_SAB_FRONT;
      msg.len = sizeof(sab_readings);
      CAN.write(msg);

      #if DEBUG
      int16_t sensor1 = ((int16_t)(msg.buf[1]) << 8) | msg.buf[0];
      int16_t sensor2 = ((int16_t)(msg.buf[3]) << 8) | msg.buf[2];
      int16_t sensor3 = ((int16_t)(msg.buf[5]) << 8) | msg.buf[4];
      int16_t sensor4 = ((int16_t)(msg.buf[7]) << 8) | msg.buf[6];
      
      Serial.println("-----------------------------");
      Serial.print("Sensor 1:\t");
      Serial.println(sensor1 / 1000);
      Serial.print("Sensor 2:\t");
      Serial.println(sensor2 / 1000);
      Serial.print("Sensor 3:\t");
      Serial.println(sensor3 / 1000);
      Serial.print("Sensor 4:\t");
      Serial.println(sensor4 / 1000);
      Serial.println();
      #endif
      
  } else if (timer_rear_update.check()) {
      CAN_LED_ticks++;
      
      sab_readings.set_sensor_1(filtered_sensor5_reading);
      sab_readings.set_sensor_2(filtered_sensor6_reading);
      sab_readings.set_sensor_3(filtered_sensor7_reading);
      sab_readings.set_sensor_4(filtered_sensor8_reading);
      sab_readings.write(msg.buf);
      msg.id = ID_SAB_REAR;
      msg.len = sizeof(sab_readings);
      CAN.write(msg);

      #if DEBUG
      int16_t sensor5 = ((int16_t)(msg.buf[1]) << 8) | msg.buf[0];
      int16_t sensor6 = ((int16_t)(msg.buf[3]) << 8) | msg.buf[2];
      int16_t sensor7 = ((int16_t)(msg.buf[5]) << 8) | msg.buf[4];
      int16_t sensor8 = ((int16_t)(msg.buf[7]) << 8) | msg.buf[6];
      
      Serial.println("-----------------------------");
      Serial.print("Sensor 5:\t");
      Serial.println(sensor5 / 1000);
      Serial.print("Sensor 6:\t");
      Serial.println(sensor6 / 1000);
      Serial.print("Sensor 7:\t");
      Serial.println(sensor7 / 1000);
      Serial.print("Sensor 8:\t");
      Serial.println(sensor8 / 1000);
      Serial.println();
      #endif
  }

  // Software filtering
  filtered_sensor1_reading = ALPHA * filtered_sensor1_reading + (1 - ALPHA) * get_sensor1_value();;
  filtered_sensor2_reading = ALPHA * filtered_sensor2_reading + (1 - ALPHA) * get_sensor2_value();;
  filtered_sensor3_reading = ALPHA * filtered_sensor3_reading + (1 - ALPHA) * get_sensor3_value();;
  filtered_sensor4_reading = ALPHA * filtered_sensor4_reading + (1 - ALPHA) * get_sensor4_value();;
  filtered_sensor5_reading = ALPHA * filtered_sensor5_reading + (1 - ALPHA) * get_sensor5_value();;
  filtered_sensor6_reading = ALPHA * filtered_sensor6_reading + (1 - ALPHA) * get_sensor6_value();;
  filtered_sensor7_reading = ALPHA * filtered_sensor7_reading + (1 - ALPHA) * get_sensor7_value();;
  filtered_sensor8_reading = ALPHA * filtered_sensor8_reading + (1 - ALPHA) * get_sensor8_value();;
}
