#include <FlexCAN.h> // import teensy library
#include <Metro.h>
#include "HyTech17.h"

// define pins
#define BOARD_LED1_PIN 10
#define BOARD_LED2_PIN 11
#define RELAY_SWITCH_PIN 12
#define EXT_BTN1_PIN A3
#define EXT_LED1_PIN A4
#define EXT_LED2_PIN A5
#define EXT_LED3_PIN A6

FlexCAN CAN(500000);
static CAN_message_t msg;

uint8_t state;
bool btn_charge_pressed = false;
bool btn_charge_debouncing = false;
uint8_t btn_charge_new;
int lastDebounceCHARGE = 0;

Metro timer_btn_charge = Metro(10);

void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
    pinMode(BOARD_LED1_PIN, OUTPUT);
    pinMode(BOARD_LED2_PIN, OUTPUT);
    pinMode(RELAY_SWITCH_PIN, OUTPUT);
    pinMode(EXT_LED1_PIN, OUTPUT);
    pinMode(EXT_LED2_PIN, OUTPUT);
    pinMode(EXT_LED3_PIN, OUTPUT);
    pinMode(EXT_BTN1_PIN, INPUT_PULLUP);
    set_state(CHARGE_STATE_NOT_CHARGING);
}

void loop () {
    while (CAN.read(msg)) {
        if (msg.id == ID_BMS_CURRENT) {
            BMS_currents bms_currents = BMS_currents(msg.buf);
            if (state == CHARGE_STATE_CHARGE_REQUESTED
                && bms_currents.getChargingState() == CHARGING) {
                set_state(CHARGE_STATE_CHARGING);
            }
        }
    }
    switch(state) {
        case CHARGE_STATE_NOT_CHARGING:
            if (btn_charge_new == lastDebounceCHARGE) {
                set_state(CHARGE_STATE_CHARGE_REQUESTED);
            }
            break;
        case CHARGE_STATE_CHARGING:
            if (btn_charge_new == lastDebounceCHARGE) {
                set_state(CHARGE_STATE_NOT_CHARGING);
            }
            break;
    }
    pollForButtonPress();
}

void pollForButtonPress() {
    /*
    * Handle start button press and depress
    */
    if (digitalRead(EXT_BTN1_PIN) == btn_charge_pressed && !btn_charge_debouncing) { // Value is different than stored
        btn_charge_debouncing = true;
        timer_btn_charge.reset();
    }
    if (btn_charge_debouncing && digitalRead(EXT_BTN1_PIN) != btn_charge_pressed) { // Value returns during debounce period
        btn_charge_debouncing = false;
    }
    if (btn_charge_debouncing && timer_btn_charge.check()) { // Debounce period finishes without value returning
        btn_charge_pressed = !btn_charge_pressed;
        if (btn_charge_pressed) {
            lastDebounceCHARGE++;
            Serial.print("Charge button pressed id ");
            Serial.println(lastDebounceCHARGE);
        }
    }
}

void set_state(uint8_t new_state) {
    if (state == new_state)
        return;
    if (new_state == CHARGE_STATE_CHARGING) {
        digitalWrite(EXT_LED1_PIN, HIGH);
    } else if (new_state == CHARGE_STATE_CHARGE_REQUESTED) {
        // send CAN Message to enable charging on BMS
    }
}
