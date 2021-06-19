#include <stdint.h>

#include "HyTech_FlexCAN.h"
#include "HyTech_CAN.h"
#include "kinetis_flexcan.h"
#include "Metro.h"

Metro timer_send_can(100);

MCU_status mcu_status{};
Dashboard_status dashboard_status{};
FlexCAN CAN(500000);

CAN_message_t tx_msg;

void setup() {
    Serial.begin(115200);

    CAN.begin();

    /* Configure CAN rx interrupt */
    interrupts();
    NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);
    attachInterruptVector(IRQ_CAN_MESSAGE,parse_can_message);
    FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;
}

void loop(){
    if (timer_send_can.check()){
        mcu_status.write(tx_msg.buf);
        tx_msg.id = ID_MCU_STATUS;
        tx_msg.len = sizeof(mcu_status);
        CAN.write(tx_msg);
    }
}

void parse_can_message(){
    static CAN_message_t rx_msg;
    static bool buzzer = false;
    while (CAN.read(rx_msg)) {
        switch (rx_msg.id) {
            case ID_DASHBOARD_STATUS:
                dashboard_status.load(rx_msg.buf);

                if (dashboard_status.get_mc_cycle_btn()){
                    buzzer = !buzzer;
                    mcu_status.set_activate_buzzer(buzzer);
                    mcu_status.write(tx_msg.buf);
                    tx_msg.id = ID_MCU_STATUS;
                    tx_msg.len = sizeof(mcu_status);
                    CAN.write(tx_msg);
                }
        }
    }
}