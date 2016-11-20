#include <FlexCAN.h>

/******* PIN definitions ***********/
#define BTN_TOGGLE 9
#define BTN_CYCLE 10
#define BTN_BOOST 11
#define BTN_START 12
#define LED_START 7
#define LED_BMS 6
#define LED_IMD 5
#define READY_SOUND 8
#define PEDAL_SIGNAL_A A3
#define PEDAL_SIGNAL_B A4
#define PEDAL_SIGNAL_C A5

/*****Dashboard States and Fault Flags********/
bool imd_fault;
bool bms_fault;
bool bspd_fault;
bool cool_fault;
bool comm_fault;
bool motor_fault;
bool pedal_fault;
bool general_fault;

/*************** BUTTON TYPES ****************
 *  Start Button
 *  Toggle Button
 *  SelectButton
*/
int count;

/**
 * CAN Variables
 */
FlexCAN can(500000);
static CAN_message_t msg;

void setup() {
    // put your setup code here, to run once:
    imd_fault = false;
    bms_fault = false;
    bspd_fault = false;
    cool_fault = false;
    comm_fault = false;
    motor_fault = false;
    pedal_fault = false;
    general_fault = false;
    Serial.begin(115200);
    can.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  pollForButtonPress();
}

void pollForButtonPress {
    // check if start button pressed
}

void toggleButtonInterrupt {

}

void startButtonInterrupt {

}
