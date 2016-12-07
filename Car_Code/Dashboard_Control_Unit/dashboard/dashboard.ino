#include <FlexCAN.h>

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
    while (can.read(msg)) {
        if (msg.id == 0x001) {
            byte fault = msg.buf[0];
            switch (fault) {
            case 1
            }
        }
    }
}
