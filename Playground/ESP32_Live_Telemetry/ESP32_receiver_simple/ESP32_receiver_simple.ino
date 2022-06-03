/**
 * @brief: Receives raw live data from the ESP32 transmitter and passes serially to the connected laptop.
 * 
 * @author: Bo Han Zhu
 * @date: 03/31/2022
 */

// ESP-Now
#include <esp_now.h>
#include <WiFi.h>

typedef struct transmit_msg_t {
  uint16_t  can_id;
  uint8_t   raw_data[8];
} transmit_msg_t;

static transmit_msg_t msg;
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len);

void setup() {
  Serial.begin(115200); // Must match baudrate of Python script that expects serial data
  
  //ESP-Now connecion
  WiFi.mode(WIFI_STA); // Set ESP32 to stationary mode

  if (esp_now_init() != ESP_OK) {
    printf("Error initializing ESP-NOW\n");
    exit(0); // Terminate program if cannot establish ESP-Now comms and wait for reset
  }
  
  esp_now_register_recv_cb(data_receive); // Register callback function
}

void loop() {
  // Not needed
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
  memcpy(&msg, incomingData, sizeof(msg));
  printf("Message ID: %u, Message Data - 0:%u 1:%u 2:%u 3:%u 4:%u 5:%u 6:%u 7:%u \n", msg.can_id, msg.raw_data[0], msg.raw_data[1], msg.raw_data[2], msg.raw_data[3], msg.raw_data[4], msg.raw_data[5], msg.raw_data[6], msg.raw_data[7]);
}
