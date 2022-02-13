#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Metro.h>

Charger_configure charger_configure;
Charger_data charger_data;

#define SHUTDOWN_A A0
#define SHUTDOWN_B A1
#define SHUTDOWN_C A2
#define SHUTDOWN_D A3
#define SHUTDOWN_E A4
#define SHUTDOWN_F A5
#define VIN_READ A6
#define WATCHDOG_OUT 7
#define SW_SHUTDOWN 6
#define CHARGE_ENABLE 5


static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
FlexCAN CAN(500000);

Metro update_ls = Metro(1000);
Metro update_CAN = Metro(100);
Metro update_watchdog = Metro(2);

int watchdog_state = 0;
void configure_charging();
void parse_can_message();
void print_data();

void setup() {
  pinMode(SHUTDOWN_A, INPUT);
  pinMode(SHUTDOWN_B, INPUT);
  pinMode(SHUTDOWN_C, INPUT);
  pinMode(SHUTDOWN_D, INPUT);
  pinMode(SHUTDOWN_E, INPUT);
  pinMode(SHUTDOWN_F, INPUT);
  pinMode(SW_SHUTDOWN, OUTPUT);
  digitalWrite(SW_SHUTDOWN, HIGH);
  
  pinMode(CHARGE_ENABLE, OUTPUT);
  digitalWrite(CHARGE_ENABLE, HIGH);

  pinMode(WATCHDOG_OUT, OUTPUT);
    
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
  if (update_CAN.check()) {
      charger_configure.write(tx_msg.buf);
      tx_msg.ext = 1;
      tx_msg.id = ID_CHARGER_CONTROL;
      tx_msg.len = sizeof(Charger_configure);
      CAN.write(tx_msg);
      tx_msg.ext = 0;

      print_data();
  }

  if (update_watchdog.check()) {
      watchdog_state = !watchdog_state;
      digitalWrite(WATCHDOG_OUT, watchdog_state);
  }

  configure_charging();
}

void configure_charging(){
  if (CHARGE_ENABLE) {
      charger_configure.set_max_charging_voltage(350);
      charger_configure.set_max_charging_current(15);
      charger_configure.set_control(1);
  } else {
      charger_configure.set_max_charging_voltage(0);
      charger_configure.set_max_charging_current(0);
      charger_configure.set_control(0);
  }
}

void parse_can_message(){
    while (CAN.read(rx_msg)) {
        rx_msg.ext = 1;
        if (rx_msg.id == ID_CHARGER_DATA) {
            charger_data.load(rx_msg.buf);
        }
        rx_msg.ext = 0;
    }
}

void print_data(){
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.print("\t\t");
  Serial.print(charger_data.get_input_ac_voltage());
  Serial.print("\t");
  Serial.print(charger_data.get_output_dc_voltage());
  Serial.print("\t");
  Serial.print(charger_data.get_output_current());
  Serial.print("\t");
  Serial.print(charger_data.get_flags());
  Serial.println();
}
