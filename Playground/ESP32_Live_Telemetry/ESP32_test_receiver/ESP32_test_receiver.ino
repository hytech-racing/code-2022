/**
 * @source: https://microcontrollerslab.com/esp32-esp-now-tutorial-arduino-ide/
 */

#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
    char character[100];
    int integer;
    float floating_value;
    bool bool_value;
} struct_message;


struct_message message;

void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&message, incomingData, sizeof(message));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(message.character);
  Serial.print("Int: ");
  Serial.println(message.integer);
  Serial.print("Float: ");
  Serial.println(message.floating_value);
  Serial.print("Bool: ");
  Serial.println(message.bool_value);
  Serial.println();
}
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(data_receive);
}
 
void loop() {

}
