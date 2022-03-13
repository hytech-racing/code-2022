![HyTech Logo](https://hytechracing.gatech.edu/images/hytech_logo_small.png)
# HyTech ESP32 Files
## Intro
This folder contains all the files needed to flash ESP32 boards for use in live telemetry.
- ESP32_get_mac_address: Run once on each ESP32 to get their MAC addresses, needed to be set in ESP32_transmitter
- ESP32_transmitter: Gets raw messages from CAN, establishes direct comms, and sends them up
- ESP32_receive: RTOS-based code to receive raw messages, call the Python C-API and the python parser to parse them, and print to serial
## Setup
1. Revert Arduino path preferences back to the standard Arduino lib instead of code-2022
2. To upload and setup Arduino IDE for ESP32, follow instructions from the link here: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
## Running List of ESP32 MAC Addresses
Because ESP-now requires MAC addresses to be directly called in the code for direct communications, here's a list to keep track:
- Transmitter MAC(s):
- Receiver MAC(s):
Please label the MAC addresses on the ESP32 boards themselves for organizational purposes. Thank you!