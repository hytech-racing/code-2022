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
  uint16_t can_id;
  uint64_t raw_data;
} transmit_msg_t;

transmit_msg_t msg;

void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status);


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
