/*
 * HyTech 2019 Charger Control Unit
 * Init 2019-05-16
 * Configured for Charger Control Board rev1
 * Controls Elcon charger and communicates with BMS over CAN to enable cell balancing
 */

#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Metro.h>

#define TOTAL_IC 8                      // Number of ICs in the system
#define CELLS_PER_IC 9                  // Number of cells per IC
#define THERMISTORS_PER_IC 3            // Number of cell thermistors per IC
#define PCB_THERM_PER_IC 2              // Number of PCB thermistors per IC

#define CHARGE_ENABLE 12
#define SHUTDOWN_B 8
#define SHUTDOWN_C 9
#define SHUTDOWN_D 10
#define WATCHDOG_OUT 13
#define TEENSY_OK 14
#define VOLTAGE_READ 16
#define IMON_B 20

#define LED A8


CCU_status ccu_status;

BMS_status bms_status;
BMS_voltages bms_voltages;

BMS_detailed_voltages bms_detailed_voltages[8][3];
BMS_temperatures bms_temperatures;
BMS_detailed_temperatures bms_detailed_temperatures[8];
BMS_onboard_detailed_temperatures bms_onboard_detailed_temperatures[TOTAL_IC];
BMS_onboard_temperatures bms_onboard_temperatures;
BMS_balancing_status bms_balancing_status[(TOTAL_IC + 3) / 4]; // Round up TOTAL_IC / 4 since data from 4 ICs can fit in a single message

int watchdog_state = 0;

static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
FlexCAN CAN(500000);

Metro timer_update_CAN = Metro(100);
Metro timer_1s = Metro(1000);

void print_cells();
void print_temps();
void parse_can_message();
void check_shutdown_signals();
void check_over_voltage_current();

void print_cells();
void print_temps();
void parse_can_message();

void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);

    pinMode(SHUTDOWN_B, INPUT);
    pinMode(SHUTDOWN_C, INPUT);
    pinMode(SHUTDOWN_D, INPUT);
    
    pinMode(CHARGE_ENABLE, OUTPUT);
    digitalWrite(CHARGE_ENABLE, HIGH);

    pinMode(WATCHDOG_OUT, OUTPUT);
    pinMode(TEENSY_OK, OUTPUT);
    digitalWrite(TEENSY_OK, HIGH);

    //prev_time = millis();

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

    ccu_status.set_charger_enabled(false);
}

void loop() {
    if (timer_update_CAN.check()) {
          ccu_status.write(tx_msg.buf);
          tx_msg.id = ID_CCU_STATUS;
          tx_msg.len = sizeof(CAN_message_ccu_status_t);
          CAN.write(tx_msg);
    } 

    if (timer_1s.check()) {
        //update serial
        print_cells();
        print_temps();
        Serial.print("Charge enable: ");
        Serial.println(ccu_status.get_charger_enabled());
        Serial.print("BMS state: ");
        Serial.println(bms_status.get_state());

        //update watchdog
        watchdog_state = !watchdog_state;
        digitalWrite(WATCHDOG_OUT, watchdog_state);
    }

    check_shutdown_signals();
    check_over_voltage_current();

}

void print_cells() {
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\t\t\t\tRaw Cell Voltages\t\t\t\t\t\t\tCell Status (Ignoring or Balancing)");
    Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8\t\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            double voltage = bms_detailed_voltages[ic][cell / 3].get_voltage(cell % 3) * 0.0001;
            Serial.print(voltage, 4); Serial.print("V\t");
        }
        Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            int balancing = bms_balancing_status[ic / 4].get_cell_balancing(ic % 4, cell);
            if (balancing) {
                Serial.print("BAL");
            }
            Serial.print("\t");
        }
        Serial.println();
    }
    Serial.println();
    Serial.println("\t\t\t\tDelta from Min Cell");
    Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            double voltage = (bms_detailed_voltages[ic][cell / 3].get_voltage(cell % 3)-bms_voltages.get_low()) * 0.0001;
            Serial.print(voltage, 4);
            Serial.print("V");
            Serial.print("\t");
        }
        Serial.println();
    }
    Serial.println();
    Serial.print("Cell voltage statistics\t\tTotal: "); Serial.print(bms_voltages.get_total() / (double) 1e2, 4); Serial.print("V\t\t");
    Serial.print("Average: "); Serial.print(bms_voltages.get_average() / (double) 1e4, 4); Serial.print("V\t\t");
    Serial.print("Min: "); Serial.print(bms_voltages.get_low() / (double) 1e4, 4); Serial.print("V\t\t");
    Serial.print("Max: "); Serial.print(bms_voltages.get_high() / (double) 1e4, 4); Serial.println("V");
}

void print_temps() {
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\t\tCell Temperatures\t\t\t\t\t\t\t\t\t   PCB Temperatures");
    Serial.println("\tTHERM 0\t\tTHERM 1\t\tTHERM 2\t\t\t\t\t\t\t\tTHERM 0\t\tTHERM 1");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int therm = 0; therm < THERMISTORS_PER_IC; therm++) {
            double temp = ((double) bms_detailed_temperatures[ic].get_temperature(therm)) / 100;
            Serial.print(temp, 2);
            Serial.print(" ºC");
            Serial.print("\t");
        }
        Serial.print("\t\t\t\t\t\t");
        for (int therm = 0; therm < PCB_THERM_PER_IC; therm++) {
            double temp = ((double) bms_onboard_detailed_temperatures[ic].get_temperature(therm)) / 100;
            Serial.print(temp, 2);
            Serial.print(" ºC");
            Serial.print("\t");
        }
        Serial.println();
    }
    Serial.print("\nCell temperature statistics\t\t Average: ");
    Serial.print(bms_temperatures.get_average_temperature() / (double) 100, 2);
    Serial.print(" ºC\t\t");
    Serial.print("Min: ");
    Serial.print(bms_temperatures.get_low_temperature() / (double) 100, 2);
    Serial.print(" ºC\t\t");
    Serial.print("Max: ");
    Serial.print(bms_temperatures.get_high_temperature() / (double) 100, 2);
    Serial.println(" ºC\n");
    Serial.print("PCB temperature statistics\t\t Average: ");
    Serial.print(bms_onboard_temperatures.get_average_temperature() / (double) 100, 2);
    Serial.print(" ºC\t\t");
    Serial.print("Min: ");
    Serial.print(bms_onboard_temperatures.get_low_temperature() / (double) 100, 2);
    Serial.print(" ºC\t\t");
    Serial.print("Max: ");
    Serial.print(bms_onboard_temperatures.get_high_temperature() / (double) 100, 2);
    Serial.println(" ºC\n");
}

/*
 * Parse incoming CAN messages
 */
void parse_can_message() {
    while (CAN.read(rx_msg)) {
        if (rx_msg.id == ID_BMS_DETAILED_TEMPERATURES) {
            BMS_detailed_temperatures temp = BMS_detailed_temperatures(rx_msg.buf);
            bms_detailed_temperatures[temp.get_ic_id()].load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_DETAILED_VOLTAGES) {
            BMS_detailed_voltages temp = BMS_detailed_voltages(rx_msg.buf);
            bms_detailed_voltages[temp.get_ic_id()][temp.get_group_id()].load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_VOLTAGES) {
            bms_voltages.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_TEMPERATURES) {
            bms_temperatures.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_ONBOARD_TEMPERATURES) {
            bms_onboard_temperatures.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_ONBOARD_DETAILED_TEMPERATURES) {
            BMS_onboard_detailed_temperatures temp = BMS_onboard_detailed_temperatures(rx_msg.buf);
            bms_onboard_detailed_temperatures[temp.get_ic_id()].load(rx_msg.buf);
        }
        
        if (rx_msg.id == ID_BMS_STATUS) {
            bms_status = BMS_status(rx_msg.buf);
            ccu_status.set_charger_enabled(bms_status.get_state() == BMS_STATE_CHARGING || bms_status.get_state() == BMS_STATE_BALANCING);
            digitalWrite(CHARGE_ENABLE, ccu_status.get_charger_enabled());
        }

        if (rx_msg.id == ID_BMS_BALANCING_STATUS) {
            BMS_balancing_status temp = BMS_balancing_status(rx_msg.buf);
            bms_balancing_status[temp.get_group_id()].load(rx_msg.buf);
        }
    }
}

void check_shutdown_signals() {
  int shutdown_b = digitalRead(SHUTDOWN_B);
  int shutdown_c = digitalRead(SHUTDOWN_C);
  int shutdown_d = digitalRead(SHUTDOWN_D);

//  if(shutdown_b == 0 || shutdown_c == 0) {
//    digitalWrite(TEENSY_OK, LOW);
//    digitalWrite(CHARGE_ENABLE, LOW);
//  }
}


//CURRENT_MAX 2.925
//VOLTAGE_MAX 14.66
//VOLTAGE_MIN 9.99
void check_over_voltage_current() {
  int imon_b = analogRead(IMON_B);
  int voltage_read = analogRead(VOLTAGE_READ);

  //assuming 10 bit resolution, 0 - 1023
 // if(imon_b > 907) {
    //digitalWrite(TEENSY_OK, LOW);
 // }

  /*if(voltage_read < 675 || voltage_read > 992) {
    digitalWrite(TEENSY_OK, LOW);
  }*/
}
