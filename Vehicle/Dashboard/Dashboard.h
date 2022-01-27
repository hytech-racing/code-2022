#pragma once

// Button pin definition
#define BTN_MARK 6
#define BTN_MODE 7
#define BTN_START A2
#define BTN_LAUNCH A3
#define BTN_MC_CYCLE 9

// LED and buzzer pin definition
#define BUZZER A4
#define LED_AMS 0
#define LED_IMD 1
#define LED_MODE 2
#define LED_MC_ERR 3
#define LED_START 4
#define SOC A5

// MCP CAN pin definition
#define CAN_CS 10

// MCP IO Expander pin definition
#define IO_CS 8

// Critical read pin definitions
#define SSOK_READ A6
#define SHUTDOWN_H_READ A7
#define INERTIA A1

#define SHUTDOWN_THERSHOLD 223 // 5V on the line

#define MCU_HEARTBEAT_TIMEOUT 1000

#define LED_MIN_FAULT 1000
