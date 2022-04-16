![HyTech Logo](https://hytechracing.gatech.edu/images/hytech_logo_small.png)
# HyTech ESP32 Files
## Intro
This folder contains all the files needed to flash ESP32 boards for use in live telemetry using the ESP-Now protocol.
Motivation: to have a live telemetry setup requiring only direct communications between MCUs, instead of going through AWS and the cloud.
ESP-Now is limited to about 200-500 meters outdoors, however, if there is Wi-Fi coverage in the area, both ESP32s can be configured to use it.

Files:
- ESP32_get_mac_address: Run once on each ESP32 to get their MAC addresses, needed to be set in ESP32_transmitter
- ESP32_transmitter: RTOS-based code to collect data from four analog sensors, read from the CAN bus, establish direct comms, and send msgs up
- ESP32_receiver: RTOS-based code to receive raw messages, call the Python C-API and the python parser to parse them, and print to serial
- ESP32_test_transmitter: Example code to send random messages to a receiver, source: https://microcontrollerslab.com/esp32-esp-now-tutorial-arduino-ide/
- ESP32_test_receiver: Example code to receive ESP-Now messages, source: https://microcontrollerslab.com/esp32-esp-now-tutorial-arduino-ide/
- ESP32_test_rear_sab_functionality: ESP32 as the rear SAB Teensy 3.2. No additional features, just analog processing and CAN functionality.
## Setup
- To upload and setup Arduino IDE for ESP32, follow instructions from the link here: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
- Ignore driver/can.h deprecation warning when verifying/uploading; the driver/twai.h library is not very mature yet. For more info: https://github.com/espressif/esp-idf/issues/5580
## Running List of ESP32 MAC Addresses
Because ESP-Now requires MAC addresses to be directly called in the code for direct communications, here's a list to keep track:
- Transmitter MAC(s):
- Receiver MAC(s):

Please label the MAC addresses on the ESP32 boards themselves for organizational purposes. Thank you!
