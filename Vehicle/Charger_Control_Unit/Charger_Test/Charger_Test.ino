#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Metro.h>

Charger_configure charger_configure;

static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
FlexCAN CAN(500000);

void setup() {
  Serial.begin(115200);
  CAN.begin();
  
  delay(1000);
}

void loop() {
  charger_configure.set_control(0);
  charger_configure.set_max_charging_current(350);
  charger_configure.set_max_charging_voltage(350);
  
  charger_configure.write(tx_msg.buf);
  tx_msg.id = ID_CHARGER_CONTROL1;
  tx_msg.len = sizeof(Charger_configure);
  CAN.write(tx_msg);

  delay(1000);
  charger_configure.set_control(1);
  charger_configure.set_max_charging_current(0);
  charger_configure.set_max_charging_voltage(0);
  
  charger_configure.write(tx_msg.buf);
  tx_msg.id = ID_CHARGER_CONTROL1;
  tx_msg.len = sizeof(Charger_configure);
  CAN.write(tx_msg);
  delay(1000);
  

}
