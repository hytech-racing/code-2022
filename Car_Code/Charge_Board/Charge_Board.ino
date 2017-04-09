#include <FlexCAN.h> // import teensy library
#include <Metro.h>
#include "HyTech17.h"

// define pins
#define BOARD_LED1_PIN 10
#define BOARD_LED2_PIN 11
#define RELAY_SWITCH_PIN 12
#define EXT_LED1_PIN A4
#define EXT_LED2_PIN A5
#define EXT_LED3_PIN A6

FlexCAN CAN(500000);
static CAN_message_t msg;

uint8_t state;

void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
    //To detect an open circuit
    //enable the pullup resistor on the Teensy input pin >>>
    pinMode(BOARD_LED1_PIN, OUTPUT);
    pinMode(BOARD_LED2_PIN, OUTPUT);
    pinMode(RELAY_SWITCH_PIN, OUTPUT);
    pinMode(EXT_LED1_PIN, OUTPUT);
    pinMode(EXT_LED2_PIN, OUTPUT);
    pinMode(EXT_LED3_PIN, OUTPUT);
    //open circuit will show a high signal outside of the working range of the sensor.
    set_state(CHARGE_STATE_NOT_CHARGING);
}

void loop () {
    
}
