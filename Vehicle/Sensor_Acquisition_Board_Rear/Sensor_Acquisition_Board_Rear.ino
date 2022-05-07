/*
 * @brief: Teensy code for the rear Sensor Acquisition Board.
 * 
 * @author: Bo Han Zhu
 * @date: 1/29/2022
 */

/*
 * Sensor Acquisition Board Rear Ports:               Teensy 3.2 Pin Number:
 * 0: Ambient Air Humidity                            A3
 * 1: Ambient Air Temperature                         A4
 * 2: Back-Left Suspension Linear Potentiometer       A5
 * 3: Back-Right Suspension Linear Potentiometer      A6
 */

#include "SAB_readings_rear.h"
#include "HyTech_FlexCAN.h"
#include "CAN_ID.h"
#include "Metro.h"

// CAN Variables
FlexCAN CAN(500000);
Metro timer_rear_update = Metro(5);
CAN_message_t msg;

// Initialize LEDs
#define CAN_LED 5

// Analog Reading
#define SENSOR_1_CHANNEL A3
#define SENSOR_2_CHANNEL A4
#define SENSOR_3_CHANNEL A5
#define SENSOR_4_CHANNEL A6
#define ALPHA 0.75                                                                                      // Adjust Filtering Strength (Higher = More Filtering)
//#define INPUT_TO_5000mV 5.9082, expression: 3.3V/(30/11)V * 3.3V/1024counts * 5V/3.3V * 1000mV/1V = 5.9082, used just for reference
// Functions scale value up by 1000, will get scaled down 1000 later in autoparse
inline float get_sensor1_value() {return 0.0;}   // Needs to be implemented for cooling loop fluid temp
inline float get_sensor2_value() {return (analogRead(SENSOR_2_CHANNEL) * -0.43003 + 190.95588) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor3_value() {return (analogRead(SENSOR_3_CHANNEL) * 0.059312 + 3.0) * 1000;}       // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor4_value() {return (analogRead(SENSOR_4_CHANNEL) * 0.059312 + 3.0) * 1000;}       // DO NOT CHANGE THIS W/O SPECIAL REASON

// Options
#define DEBUG (true)

SAB_readings_rear sab_readings_rear;

/*
 * Variables to store filtered values
 */
float filtered_sensor1_reading{};
float filtered_sensor2_reading{};
float filtered_sensor3_reading{};
float filtered_sensor4_reading{};


void setup() {
  //Initiallizes CAN
  pinMode(CAN_LED, OUTPUT);
  CAN.begin();

  //Get initial readings
  filtered_sensor1_reading = get_sensor1_value();
  filtered_sensor2_reading = get_sensor2_value();
  filtered_sensor3_reading = get_sensor3_value();
  filtered_sensor4_reading = get_sensor4_value();
}

void loop() {  
  if (timer_rear_update.check()){
    
      digitalWrite(CAN_LED, !digitalRead(CAN_LED));
      
      sab_readings_rear.set_sensor_1(filtered_sensor1_reading);
      sab_readings_rear.set_sensor_2(filtered_sensor2_reading);
      sab_readings_rear.set_sensor_3(filtered_sensor3_reading);
      sab_readings_rear.set_sensor_4(filtered_sensor4_reading);
      sab_readings_rear.write(msg.buf);
      msg.id = ID_SAB_READINGS_REAR;
      msg.len = sizeof(sab_readings_rear);
      CAN.write(msg);

      #if DEBUG      
      Serial.println("-----------------------------");
      Serial.print("Sensor 1:\t");
      Serial.println(filtered_sensor1_reading / 1000.0);
      Serial.print("Sensor 2:\t");
      Serial.println(filtered_sensor2_reading / 1000.0);
      Serial.print("Sensor 3:\t");
      Serial.println(filtered_sensor3_reading / 1000.0);
      Serial.print("Sensor 4:\t");
      Serial.println(filtered_sensor4_reading / 1000.0);
      Serial.println();
      #endif
      
  }

  // Software filtering
  filtered_sensor1_reading = ALPHA * filtered_sensor1_reading + (1 - ALPHA) * get_sensor1_value();;
  filtered_sensor2_reading = ALPHA * filtered_sensor2_reading + (1 - ALPHA) * get_sensor2_value();;
  filtered_sensor3_reading = ALPHA * filtered_sensor3_reading + (1 - ALPHA) * get_sensor3_value();;
  filtered_sensor4_reading = ALPHA * filtered_sensor4_reading + (1 - ALPHA) * get_sensor4_value();;
}
