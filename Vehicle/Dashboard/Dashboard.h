#pragma once

// Button pin definition
#define BTN_MARK A0
#define BTN_MODE A1
#define BTN_MC_CYCLE A2
#define BTN_START A3
#define BTN_LC A4

// LED and buzzer pin definition
#define BUZZER A5
#define LED_AMS 0
#define LED_IMD 1
#define LED_MODE 2
#define LED_MC_ERR 3
#define LED_START 4

// MCP CAN pin definition
#define SPI_CS 10

#define SSOK_READ A6
#define SHUTDOWN_H_READ A7

#define SHUTDOWN_THERSHOLD 223 // 5V on the line

#define MCU_HEARTBEAT_TIMEOUT 1000

#define LED_MIN_FAULT 1000
