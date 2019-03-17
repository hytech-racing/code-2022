#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>

BMS_status bms_status;
BMS_voltages bms_voltages;

static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
FlexCAN CAN(500000);

void setup() {

    Serial.begin(115200);
    CAN.begin();

    /* Configure CAN rx interrupt */
    interrupts();
    NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);
    attachInterruptVector(IRQ_CAN_MESSAGE,parse_can_message);
    FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;
    /* Configure CAN rx interrupt */

    delay(1000);

     Serial.println("CAN system and serial communication initialized");
}

void loop() {
    Serial.print("BMS State: ");
    Serial.println(bms_status.get_state());
    Serial.print("BMS VOLTAGE AVERAGE: ");
    Serial.println(bms_voltages.get_average() / (double) 10000);
    Serial.print("BMS VOLTAGE LOW: ");
    Serial.println(bms_voltages.get_low() / (double) 10000);
    Serial.print("BMS VOLTAGE HIGH: ");
    Serial.println(bms_voltages.get_high() / (double) 10000);
    Serial.print("BMS VOLTAGE TOTAL: ");
    Serial.println(bms_voltages.get_total() / (double) 100);
    delay(500);
}

/*
 * Parse incoming CAN messages
 */
void parse_can_message() {
    while (CAN.read(rx_msg)) {
        if (rx_msg.id == ID_BMS_STATUS) {
            bms_status.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_VOLTAGES) {
            bms_voltages.load(rx_msg.buf);
        }
    }
}
