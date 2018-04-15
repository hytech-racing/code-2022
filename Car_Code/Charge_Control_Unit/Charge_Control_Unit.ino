/*
 * HyTech 2018 Vehicle Charge Control Unit
 */

#include <FlexCAN.h>
#include "HyTech17.h"
#include <Metro.h>

#define CHARGE_ENABLE 10

/*
 * Global variables
 */
CCU_status ccu_status;

FlexCAN CAN(500000);
static CAN_message_t msg;
Metro timer_update_CAN = Metro(100);

void setup() {
    pinMode(CHARGE_ENABLE, OUTPUT);

    Serial.begin(115200); // Init serial for PC communication
    CAN.begin(); // Init CAN for vehicle communication
    delay(100);
    Serial.println("CAN system and serial communication initialized");

    ccu_status.set_charger_enabled(false);
}

void loop() {
    /*
     * Handle incoming CAN messages
     */
    while (CAN.read(msg)) {
        if (msg.id == ID_BMS_STATUS) {
            BMS_status bms_status = BMS_status(msg.buf);
            ccu_status.set_charger_enabled(bms_status.get_state() == BMS_STATE_BALANCING_CHARGE_ENABLE ||
                                            bms_status.get_state() == BMS_STATE_CHARGING);
        }
    }


    if (ccu_status.get_charger_enabled()) {
        digitalWrite(CHARGE_ENABLE, HIGH);
    } else {
        digitalWrite(CHARGE_ENABLE, LOW);
    }
    
    if (timer_update_CAN.check()) {
        ccu_status.write(msg.buf);
        msg.id = ID_CCU_STATUS;
        msg.len = 1;
        CAN.write(msg);
    }
}



