/**
 * @brief: Functions as the rear SAB to handle four analog sensors and send all CAN msgs via ESP-Now to the receiver.
 * 
 * @author: Bo Han Zhu
 * @date: 04/06/2022
 */
 
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Variables and Imports
/////////////////////////////////////////////////////////////////////////////////////////////////////

// FreeRTOS
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define QUEUE_LEN 255
#define msg_queue_led_pin LED_BUILTIN // TODO: Replace with pin to control external LED
static QueueHandle_t msg_queue;
static SemaphoreHandle_t mutex; // The serial monitor is a shared resource, need a mutex to lock between task-sharing

// ESP-Now
#include <esp_now.h>
#include <WiFi.h>

// TODO: REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x7C, 0x9E, 0xBD, 0x37, 0xCA, 0x84};

typedef struct transmit_msg_t {
  uint16_t  can_id;
  uint8_t   raw_data[8];
} transmit_msg_t;

transmit_msg_t msg;

void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status);

// CAN bus
// Reference: https://docs.espressif.com/projects/esp-idf/en/release-v3.3/api-reference/peripherals/can.html
#include "driver/gpio.h"
#include "driver/can.h"
#define CAN_TX GPIO_NUM_17
#define CAN_RX GPIO_NUM_16

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Task Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Takes messages from the queue and transmits them via ESP-Now
 */
void transmit_message(void * no_params) {
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    Serial.println("Error initializing ESP-NOW");
    xSemaphoreGive(mutex);
    return;
  }

  esp_now_register_send_cb(data_sent);
  
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;     
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    xSemaphoreTake(mutex, portMAX_DELAY);
    Serial.println("Failed to add peer");
    xSemaphoreGive(mutex);
    return;
  }

  while (true) {
    if (xQueueReceive(msg_queue, (void *)&msg, 0) == pdTRUE) {
      // Determines outcome of transaction
      esp_err_t outcome = esp_now_send(broadcastAddress, (uint8_t *) &msg, sizeof(msg));
      
      xSemaphoreTake(mutex, portMAX_DELAY);
      if (outcome == ESP_OK) Serial.println("Mesage sent successfully!");
      else Serial.println("Error sending the message");
      xSemaphoreGive(mutex);
    }
  }
}

/**
 * @brief: Toggles LED if there is a message in the queue
 *         Waits 100 ms before checking again if LED is toggled
 */
void blink_msg_queue_led(void * no_params) {
  pinMode(msg_queue_led_pin, OUTPUT);

  while (true) {
    if (uxQueueMessagesWaiting(msg_queue) > 0)
    {
      digitalWrite(msg_queue_led_pin, HIGH);
    }
    else
    {
      digitalWrite(msg_queue_led_pin, LOW);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief: Spies on the CAN bus and adds incoming messages to the message queue
 */
void receive_can_messages(void * no_params) {
  //Initialize configuration structures using macro initializers
  can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, CAN_MODE_NORMAL);
  can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
  can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

  //Install CAN driver
  xSemaphoreTake(mutex, portMAX_DELAY);
  if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
      printf("Driver installed\n");
  } else {
      printf("Failed to install driver\n");
      return;
  }
  xSemaphoreGive(mutex);

  //Start CAN driver
  xSemaphoreTake(mutex, portMAX_DELAY);
  if (can_start() == ESP_OK) {
    Serial.println("Driver started");
  } else {
    Serial.println("Failed to start driver");
    return;
  }
  xSemaphoreGive(mutex);

  while (true) {
    can_message_t can_message;
    
    if (can_receive(&can_message, portMAX_DELAY) == ESP_OK) {
      // Print out incoming can message to console
      xSemaphoreTake(mutex, portMAX_DELAY);
      Serial.print("ID is " + String(can_message.identifier) + ", Data: ");
      for (int i = 0; i < can_message.data_length_code; i++) {
        Serial.print(String(can_message.data[i]));
      }
      Serial.println();
      xSemaphoreGive(mutex);

      // Process received message
      transmit_msg_t incoming_can_message;
      incoming_can_message.can_id = can_message.identifier;
      for (int i = 0; i < 8; i++) {
        incoming_can_message.raw_data[i] = (i < can_message.data_length_code) ? can_message.data[i] : 0u;
      }

      // Add to queue
      xQueueSend(msg_queue, (void *) &incoming_can_message, portMAX_DELAY);
    }
  } // end while
}

/**
 * @brief: Entry point to FreeRTOS framework
 */
void setup() {
  // Initialize resources
  Serial.begin(115200);
  msg_queue = xQueueCreate(QUEUE_LEN, sizeof(transmit_msg_t));
  mutex = xSemaphoreCreateMutex();

  // Create tasks
  xTaskCreatePinnedToCore(
    transmit_message,
    "Transmit Message",
    2048,
    NULL,
    5,// Highest Priority Task
    NULL,
    app_cpu);
  
  xTaskCreatePinnedToCore(
    receive_can_messages,
    "Receive CAN Messages",
    2048,
    NULL,
    4,
    NULL,
    app_cpu);
  
  xTaskCreatePinnedToCore(
    blink_msg_queue_led,
    "Blink Message Queue LED",
    1024,
    NULL,
    1, // Lowest Priority Task
    NULL,
    app_cpu);

  vTaskDelete(NULL); // Delete setup() and loop() task
}

void loop() {
  // Not needed in FreeRTOS Implementation
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief: Copies in the received raw message and adds it to the raw message queue
 * 
 * @param[in]: mac_addr - pointer to the MAC address
 * @param[in]: status - whether the ESP-Now message successfully transmitted or not
 */
void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  Serial.print("\r\nStatus of Last Message Sent:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  xSemaphoreGive(mutex);
}
