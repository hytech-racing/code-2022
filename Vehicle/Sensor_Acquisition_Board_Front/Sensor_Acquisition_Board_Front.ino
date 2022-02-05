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
#include <Adafruit_GPS.h>
// CAN Variables
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> CAN_IMU;      //Pins to IMU are A8 and A9 (22 and 23), which is CAN1
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_Vehicle;  //Pins to the Vehicle CanBus are D0 and D1 (0 and 1), which is CAN2
CAN_message_t msg;
unsigned char len = 0;
unsigned char buf[8];
SAB_readings_front sab_readings_front;
// SAB_readings_gps sab_readings_gps; // what is this?

// SAB Analog Readings and Filtering
#define SENSOR_1_CHANNEL A0
#define SENSOR_2_CHANNEL A1
#define ALPHA 0.75                                                                                      // Adjust Filtering Strength (Higher = More Filtering)
inline float get_sensor1_value() {return (analogRead(SENSOR_1_CHANNEL) * 0.059312 + 3.0) * 1000;}       // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor2_value() {return (analogRead(SENSOR_2_CHANNEL) * 0.059312 + 3.0) * 1000;}       // DO NOT CHANGE THIS W/O SPECIAL REASON
float filtered_sensor1_reading{};
float filtered_sensor2_reading{};
#define GPSSerial Serial2;

Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false

// Timers
Metro timer_SAB_front = Metro(200);
Metro timer_IMU = Metro(200);
Metro timer_adafruit_gps = Metro(200);

void swap_bytes(uint8_t *low_byte, uint8_t high_byte);

// Initialize LEDs
#define IMU_LED 5
#define VEHICLE_LED 6

// Options
#define DEBUG (false)
#define ZERO_IMU (false)
#define GPSECHO (false)

void setup()
{
  // Initialize both sets of CAN lines
  CAN_IMU.begin();
  CAN_IMU.setBaudRate(1000000); // IMU CAN speed is 1Mbps
  CAN_Vehicle.begin();
  CAN_Vehicle.setBaudRate(500000); // Vehicle CAN speed is 500kbps

  // Set LED pinmodes
  pinMode(IMU_LED, OUTPUT);
  pinMode(VEHICLE_LED, OUTPUT);

  // Zero IMU
  #if ZERO_IMU
  CAN_message_t zero_msg;
  zero_msg.id = 0x10;
  zero_msg.len = 8;
  uint8_t zero_buf[8] = {0x69, 0x6d, 0x75, 0x7a, 0x65, 0x72, 0x6f, 0x7a};
  for (int i = 0; i < 8; i++)
  {
    buf[i] = zero_buf[i];
    }
    CAN_IMU.write(zero_msg);
    delay(6000); // delay 6 seconds to perform zeroing
  #endif

  #if DEBUG
      Serial.begin(9600);
      Serial.println("CAN INIT OK!");
  #endif

    // Get initial analog sensor readings
    filtered_sensor1_reading = get_sensor1_value();
    filtered_sensor2_reading = get_sensor2_value();

  // GPS testing
  #if GPSECHO

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  // GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Uncomment to request updates on antenna status
  // GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);
#endif
}

void loop() {

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
    sab_readings_front.write(msg.buf);
    msg.id = ID_SAB_READINGS_FRONT;
    msg.len = sizeof(sab_readings_front);
    CAN_Vehicle.write(msg);
  }
  
  // Read and write CAN message
  if (timer_IMU.check() && CAN_IMU.read(msg)) {
    digitalWrite(VEHICLE_LED, !digitalRead(VEHICLE_LED)); // Invert LED status to simulate flashing
    digitalWrite(IMU_LED, !digitalRead(IMU_LED));         // Invert LED status to simulate flashing
    
    #if DEBUG
    // accelerometer
    if (msg.id == ID_IMU_ACCELEROMETER) {
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
    if (msg.id == ID_IMU_GYROSCOPE) {
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
  }

  // Software analog filtering
  filtered_sensor1_reading = ALPHA * filtered_sensor1_reading + (1 - ALPHA) * get_sensor1_value();;
  filtered_sensor2_reading = ALPHA * filtered_sensor2_reading + (1 - ALPHA) * get_sensor2_value();;

// GPS testing
#if GPSECHO
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c)
      Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived())
  {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    Serial.print(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return;                       // we can fail to parse a sentence in which case we should just wait for another
  }

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer_adafruit_gps > 2000)
  {
    timer_adafruit_gps = millis(); // reset the timer
    Serial.print("\nTime: ");
    if (GPS.hour < 10)
    {
      Serial.print('0');
    }
    Serial.print(GPS.hour, DEC);
    Serial.print(':');
    if (GPS.minute < 10)
    {
      Serial.print('0');
    }
    Serial.print(GPS.minute, DEC);
    Serial.print(':');
    if (GPS.seconds < 10)
    {
      Serial.print('0');
    }
    Serial.print(GPS.seconds, DEC);
    Serial.print('.');
    if (GPS.milliseconds < 10)
    {
      Serial.print("00");
    }
    else if (GPS.milliseconds > 9 && GPS.milliseconds < 100)
    {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC);
    Serial.print('/');
    Serial.print(GPS.month, DEC);
    Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: ");
    Serial.print((int)GPS.fix);
    Serial.print(" quality: ");
    Serial.println((int)GPS.fixquality);
    if (GPS.fix)
    {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4);
      Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4);
      Serial.println(GPS.lon);
      Serial.print("Speed (knots): ");
      Serial.println(GPS.speed);
      Serial.print("Angle: ");
      Serial.println(GPS.angle);
      Serial.print("Altitude: ");
      Serial.println(GPS.altitude);
      Serial.print("Satellites: ");
      Serial.println((int)GPS.satellites);
    }
  }
#endif
}

void swap_bytes(uint8_t *low_byte, uint8_t *high_byte) {
  uint8_t temp = *low_byte;
  *low_byte = *high_byte;
  *high_byte = temp;
}
