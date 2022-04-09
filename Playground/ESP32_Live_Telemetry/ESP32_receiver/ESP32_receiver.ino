/**
 * @brief: Receives raw live data from the ESP32 transmitter and passes serially to the connected laptop.
 * 
 * @author: Bo Han Zhu
 * @date: 03/31/2022
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
#define led_pin LED_BUILTIN
static QueueHandle_t msg_queue;

// ESP-Now
#include <esp_now.h>
#include <WiFi.h>

typedef struct transmit_msg_t {
  uint16_t  can_id;
  uint8_t   raw_data[8];
} transmit_msg_t;

static transmit_msg_t incoming_msg;
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len);

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Task Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief: Receives messages transmitted via ESP-Now and adds to the raw message queue.
 */
void add_incoming_msg(void * no_params) {
  //ESP-Now connecion
  WiFi.mode(WIFI_STA); // Set ESP32 to stationary mode

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    exit(0); // Terminate program if cannot establish ESP-Now comms and wait for reset
  }

  esp_now_register_recv_cb(data_receive); // Register callback function
}

/**
 * @brief: Spies on the message queue and sends to serial
 */
void print_to_serial(void * no_params) {
  transmit_msg_t msg;

  while (true) {
    // If message exists in the queue, pass to serial "can_id,raw_data"
    if (xQueueReceive(msg_queue, (void *)&msg, 0) == pdTRUE) {
      Serial.print(msg.can_id + "," + msg.raw_data[0] + msg.raw_data[1] + msg.raw_data[2] + msg.raw_data[3]
                                    + msg.raw_data[4] + msg.raw_data[5] + msg.raw_data[6] + msg.raw_data[7]);
    }
  }
}

/**
 * @brief: Toggles LED if there is a message in the queue
 *         Waits 100 ms before checking again if LED is toggled
 */
void blink_led(void * no_params) {
  pinMode(led_pin, OUTPUT);

  while (true) {
    if (uxQueueMessagesWaiting(msg_queue) > 0)
    {
      digitalWrite(led_pin, HIGH);
    }
    else
    {
      digitalWrite(led_pin, LOW);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief: Entry point to FreeRTOS framework
 */
void setup() {
  //Initialize resources
  Serial.begin(115200); // Must match baudrate of Python script that expects serial data
  msg_queue = xQueueCreate(QUEUE_LEN, sizeof(transmit_msg_t));

  // Create tasks
  xTaskCreatePinnedToCore(
    add_incoming_msg,
    "Add Incoming Message",
    2048,
    NULL,
    3,// Highest Priority Task
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    print_to_serial,
    "Print to Serial",
    1024,
    NULL,
    2, 
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    blink_led,
    "Blink LED",
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
 * @param[in]: mac - pointer to the MAC address, automatically filled in and not used
 * @param[in]: incomingData - pointer to the data, automatically filled in
 * @param[in]: len - length of the data received, automatically filled in and not used
 */
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incoming_msg, incomingData, sizeof(transmit_msg_t));  // Copy in data
  xQueueSend(msg_queue, (void *) &incoming_msg, portMAX_DELAY);
}
