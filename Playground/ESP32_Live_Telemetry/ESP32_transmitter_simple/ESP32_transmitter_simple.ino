/*
 * @brief: ESP32 test code for the rear Sensor Acquisition Board.
 *         Only provides existing rear SAB functionality, i.e. CAN comms and sensor reading.
 * 
 * @author: Bo Han Zhu
 * @date: 04/16/2022
 */

/*
 * Sensor Acquisition Board Rear Ports:               Adafruit Huzzah32 Pin Number:
 * 0: Cooling Loop Fluid Temperature                  A7 (GPIO_NUM_32)
 * 1: Ambient Air Temperature                         A4 (GPIO_NUM_36)
 * 2: Back-Left Suspension Linear Potentiometer       A3 (GPIO_NUM_39)
 * 3: Back-Right Suspension Linear Potentiometer      A2 (GPIO_NUM_34)
 */

#include "SAB_readings_rear.h"
#include "CAN_ID.h"
#include "Metro.h"
#include "driver/can.h"
#include "driver/gpio.h"

// CAN Variables
#define CAN_TX GPIO_NUM_17
#define CAN_RX GPIO_NUM_16
Metro timer_rear_update = Metro(200);
can_message_t can_message_rx;
can_message_t can_message_tx;
SAB_readings_rear sab_readings_rear;

// Pin definitions
#define CAN_LED          GPIO_NUM_13
#define SENSOR_1_CHANNEL GPIO_NUM_32
#define SENSOR_2_CHANNEL GPIO_NUM_36
#define SENSOR_3_CHANNEL GPIO_NUM_39
#define SENSOR_4_CHANNEL GPIO_NUM_34
#define ALPHA 0.75  // Adjust Filtering Strength (Higher = More Filtering)
// #define INPUT_TO_5000mV 5.9082, expression: 3.3V/(30/11)V * 3.3V/1024counts * 5V/3.3V * 1000mV/1V = 5.9082, used just for reference
// Functions scale value up by 1000, will get scaled down 1000 later in parser
#define TICKS_4096_TO_1024 (4096 / 1024)         // ESP32 ADC translates to 4096 ticks per 3.3V instead of Teensy's 1024 ticks per 3.3V
inline float get_sensor1_value() {return 0.0;}   // Needs to be implemented for cooling loop fluid temp
inline float get_sensor2_value() {return 0.0;}   // Ambient Air Temperature sensor was lost :(
// inline float get_sensor2_value() {return (analogRead(SENSOR_2_CHANNEL) * -0.43003 + 190.95588) * 1000;} // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor3_value() {return (analogRead(SENSOR_3_CHANNEL) / TICKS_4096_TO_1024 * 0.059312 + 3.0) * 1000;}       // DO NOT CHANGE THIS W/O SPECIAL REASON
inline float get_sensor4_value() {return (analogRead(SENSOR_4_CHANNEL) / TICKS_4096_TO_1024 * 0.059312 + 3.0) * 1000;}       // DO NOT CHANGE THIS W/O SPECIAL REASON

// Options
#define DEBUG (true)

/*
 * Variables to store filtered values
 */
float filtered_sensor1_reading{};
float filtered_sensor2_reading{};
float filtered_sensor3_reading{};
float filtered_sensor4_reading{};

/**
 * @brief: ESP-Now
 */

#include <esp_now.h>
#include <WiFi.h>

// TODO: REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x7C, 0x9E, 0xBD, 0x47, 0x5A, 0x14};

#define QUEUE_LED GPIO_NUM_11

typedef struct transmit_msg_t {
  uint16_t  can_id;
  uint8_t   raw_data[8];
} transmit_msg_t;

transmit_msg_t msg;

void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status);


void setup() {
  pinMode(CAN_LED, OUTPUT);

  //Initialize configuration structures using macro initializers
  can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, CAN_MODE_NORMAL);
  can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
  can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

  //Install CAN driver
  if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
      printf("Driver installed\n");
  } else {
      printf("Failed to install driver\n");
      return;
  }

  //Start CAN driver
  if (can_start() == ESP_OK) {
    Serial.println("Driver started");
    digitalWrite(CAN_LED, HIGH);
  } else {
    Serial.println("Failed to start driver");
    digitalWrite(CAN_LED, LOW);
    return;
  }

  // Start ESP-NOW
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(data_sent);
  
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;     
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  //Get initial readings
  filtered_sensor1_reading = get_sensor1_value();
  filtered_sensor2_reading = get_sensor2_value();
  filtered_sensor3_reading = get_sensor3_value();
  filtered_sensor4_reading = get_sensor4_value();
}

void loop() { 

  if (timer_rear_update.check()){
    // Set SAB readings rear
    sab_readings_rear.set_sensor_1(filtered_sensor1_reading);
    sab_readings_rear.set_sensor_2(filtered_sensor2_reading);
    sab_readings_rear.set_sensor_3(filtered_sensor3_reading);
    sab_readings_rear.set_sensor_4(filtered_sensor4_reading);

    sab_readings_rear.write(can_message_tx.data);
    
    can_message_tx.identifier = ID_SAB_READINGS_REAR;
    can_message_tx.flags = CAN_MSG_FLAG_NONE;
    can_message_tx.data_length_code = sizeof(sab_readings_rear);
    can_transmit(&can_message_tx, portMAX_DELAY);

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
  filtered_sensor1_reading = ALPHA * filtered_sensor1_reading + (1 - ALPHA) * get_sensor1_value();
  filtered_sensor2_reading = ALPHA * filtered_sensor2_reading + (1 - ALPHA) * get_sensor2_value();
  filtered_sensor3_reading = ALPHA * filtered_sensor3_reading + (1 - ALPHA) * get_sensor3_value();
  filtered_sensor4_reading = ALPHA * filtered_sensor4_reading + (1 - ALPHA) * get_sensor4_value();

  // Handle incoming CAN messages
  if (can_receive(&can_message_rx, portMAX_DELAY) == ESP_OK) {
    // Print out incoming can message to console
    Serial.print("ID is " + String(can_message_rx.identifier) + ", Data: ");
    for (int i = 0; i < can_message_rx.data_length_code; i++) {
      Serial.print(String(can_message_rx.data[i]));
    }
    Serial.println();

    // Process received message
    transmit_msg_t incoming_can_message;
    incoming_can_message.can_id = can_message_rx.identifier;
    for (int i = 0; i < 8; i++) {
      incoming_can_message.raw_data[i] = (i < can_message_rx.data_length_code) ? can_message_rx.data[i] : 0u;
    }

    esp_err_t outcome = esp_now_send(broadcastAddress, (uint8_t *) &msg, sizeof(msg));
    
    if (outcome == ESP_OK) Serial.println("Mesage sent successfully!");
    else Serial.println("Error sending the message");
  }
}

/**
 * @brief: Copies in the received raw message and adds it to the raw message queue
 * 
 * @param[in]: mac_addr - pointer to the MAC address
 * @param[in]: status - whether the ESP-Now message successfully transmitted or not
 */
void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nStatus of Last Message Sent:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
