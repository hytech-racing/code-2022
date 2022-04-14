#pragma once

// Button pin definition
#define BTN_MARK 6
#define BTN_MODE 7
#define BTN_START A2
#define BTN_LC A3
#define BTN_MC_CYCLE 9

// LED and buzzer pin definition
#define BUZZER A4
#define LED_AMS 0
#define LED_IMD 1
#define LED_MODE 2
#define LED_MC_ERR 3
#define LED_START 4
#define LED_INERTIA A5

// MCP CAN pin definition
#define CAN_CS 10

// MCP IO Expander pin definition
#define IO_CS 8
#define IO_ADDR 0

// Critical read pin definitions
#define SSOK_READ A6
#define SHUTDOWN_H_READ A7
#define INERTIA_READ A1

// 7-segment Fault Codes
#define AMS_SEG 1
#define IMD_SEG 2
#define MC_GATE_SEG 3
#define MC_UNDER_VOLTAGE_SEG 4
#define MC_OVER_SPEED_SEG 5
#define MC_OVER_TEMP_SEG 6
#define MC_GENERAL_SEG 7
#define INERTIA_SEG 8

// IO Expander Number Encodings
// 1 = AMS, 2 = IMD, 3 = MC Run Gate Fault, 4 = MC Run Undervoltage, 5 = MC Run Overspeed, 6 = MC Run Overtemp, 7 = MC General, 8 = Inertia ..., 10 = default off state
uint8_t number_encodings[11] = {0b01000000, 0b01111001, 0b00100100, 0b00110000, 0b00011001, 0b00010010, 0b00000010, 0b01111000, 0b10000000, 0b00011000, 0b11111111};
uint8_t display_list[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };

#define SHUTDOWN_THERSHOLD 223 // 5V on the line

#define MCU_HEARTBEAT_TIMEOUT 1000

#define LED_MIN_FAULT 1000
