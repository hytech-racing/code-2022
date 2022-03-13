/**
 * @brief: Sketch to get the MAC Address of an ESP32 board, referenced from: https://microcontrollerslab.com/esp32-esp-now-tutorial-arduino-ide/
 * 
 * @author: Bo Han Zhu
 * @date: 3/13/2022
 */

#include "WiFi.h"
 
void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
}
 
void loop(){

}
