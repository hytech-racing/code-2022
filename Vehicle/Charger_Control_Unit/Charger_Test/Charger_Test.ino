#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Metro.h>


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
#define CELLS_PER_EVEN_IC 12
#define THERMISTORS_PER_IC 4
#define TOTAL_IC 8

Charger_configure charger_configure;
Charger_data charger_data;
BMS_detailed_voltages bms_detailed_voltages[TOTAL_IC][4];
BMS_detailed_temperatures bms_detailed_temperatures[TOTAL_IC][2];

static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
FlexCAN CAN(500000);

Metro update_ls = Metro(1000);
Metro update_CAN = Metro(1000);
Metro update_watchdog = Metro(2);

int watchdog_state = 0;
void configure_charging();
void parse_can_message();
void print_data();
void print_voltages();
void print_temperatures();

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
      print_voltages();
      print_temperatures();
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
        if (rx_msg.id == ID_BMS_DETAILED_TEMPERATURES) {
            BMS_detailed_temperatures temp = BMS_detailed_temperatures(rx_msg.buf);
            bms_detailed_temperatures[temp.get_ic_id()][temp.get_group_id()].load(rx_msg.buf);
            
        }

        if (rx_msg.id == ID_BMS_DETAILED_VOLTAGES) {
            BMS_detailed_voltages temp = BMS_detailed_voltages(rx_msg.buf);
            bms_detailed_voltages[temp.get_ic_id()][temp.get_group_id()].load(rx_msg.buf);
        }
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

void print_voltages(){
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.println("\t\t\t\tRaw Cell Voltages\t\t\t\t\t\t\tCell Status (Ignoring or Balancing)");
  Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8\t\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
  for (int ic = 0; ic < TOTAL_IC; ic++) {
      Serial.print("IC"); Serial.print(ic); Serial.print("\t");
      for (int cell = 0; cell < CELLS_PER_EVEN_IC; cell++) {
          double voltage = bms_detailed_voltages[ic][cell / 3].get_voltage(cell % 3);
          Serial.print(voltage, 4); Serial.print("V\t");
      }
      //Serial.print("\t");
      //for (int cell = 0; cell < CELLS_PER_EVEN_IC; cell++) {
         // int balancing = bms_balancing_status[ic / 4].get_cell_balancing(ic % 4, cell);
          //if (balancing) {
            //  Serial.print("BAL");
       //   }
        //  Serial.print("\t");
      //}
      Serial.println();
  }
  Serial.println();
  //Serial.println("\t\t\t\tDelta from Min Cell");
  //Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
  //for (int ic = 0; ic < TOTAL_IC; ic++) {
      //Serial.print("IC"); Serial.print(ic); Serial.print("\t");
     // for (int cell = 0; cell < CELLS_PER_EVEN_IC; cell++) {
       //   double voltage = (bms_detailed_voltages[ic][cell / 3].get_voltage(cell % 3)-bms_voltages.get_low()) * 0.0001;
       //   Serial.print(voltage, 4);
        //  Serial.print("V");
       //   Serial.print("\t");
      //}
    //  Serial.println();
  //}
  Serial.println();
//  Serial.print("Cell voltage statistics\t\tTotal: "); Serial.print(bms_voltages.get_total() / (double) 1e2, 4); Serial.print("V\t\t");
 // Serial.print("Average: "); Serial.print(bms_voltages.get_average() / (double) 1e4, 4); Serial.print("V\t\t");
 // Serial.print("Min: "); Serial.print(bms_voltages.get_low() / (double) 1e4, 4); Serial.print("V\t\t");
 // Serial.print("Max: "); Serial.print(bms_voltages.get_high() / (double) 1e4, 4); Serial.println("V");
}

void print_temperatures(){
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\t\tCell Temperatures\t\t\t\t\t\t\t\t\t   PCB Temperatures");
    Serial.println("\tTHERM 0\t\tTHERM 1\t\tTHERM 2\t\t\t\t\t\t\t\tTHERM 0\t\tTHERM 1");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int therm = 0; therm < THERMISTORS_PER_IC; therm++) {
            double temp = ((double) bms_detailed_temperatures[ic][therm / 2].get_temperature(therm % 3));
            Serial.print(temp, 2);
            Serial.print(" ºC");
            Serial.print("\t");
        }
        Serial.print("\t\t\t\t\t\t");
       // for (int therm = 0; therm < PCB_THERM_PER_IC; therm++) {
        //    double temp = ((double) bms_onboard_detailed_temperatures[ic].get_temperature(therm)) / 100;
        //    Serial.print(temp, 2);
        //    Serial.print(" ºC");
       //     Serial.print("\t");
        //}
        Serial.println();
    }
    //Serial.print("\nCell temperature statistics\t\t Average: ");
   // Serial.print(bms_temperatures.get_average_temperature() / (double) 100, 2);
   // Serial.print(" ºC\t\t");
   // Serial.print("Min: ");
   // Serial.print(bms_temperatures.get_low_temperature() / (double) 100, 2);
   // Serial.print(" ºC\t\t");
   // Serial.print("Max: ");
 //   Serial.print(bms_temperatures.get_high_temperature() / (double) 100, 2);
  // Serial.println(" ºC\n");
   // Serial.print("PCB temperature statistics\t\t Average: ");
   // Serial.print(bms_onboard_temperatures.get_average_temperature() / (double) 100, 2);
  //  Serial.print(" ºC\t\t");
  //  Serial.print("Min: ");
 //   Serial.print(bms_onboard_temperatures.get_low_temperature() / (double) 100, 2);
   // Serial.print(" ºC\t\t");
   // Serial.print("Max: ");
  //  Serial.print(bms_onboard_temperatures.get_high_temperature() / (double) 100, 2);
   // Serial.println(" ºC\n");
}
