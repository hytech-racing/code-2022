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
#define CELLS_PER_EVEN_IC 12                  // Number of cells per IC
#define CELLS_PER_ODD_IC 9
#define THERMISTORS_PER_IC 4            // Number of cell thermistors per IC
#define PCB_THERM_PER_IC 2              // Number of PCB thermistors per IC
#define MAX_CURRENT 5

#define SHUTDOWN_A A0
#define SHUTDOWN_B A1
#define SHUTDOWN_C A2
#define SHUTDOWN_D A3
#define SHUTDOWN_E A4
#define SHUTDOWN_F A5
#define VIN_READ A6
#define WATCHDOG_OUT 7
#define TEENSY_OK 6
#define STATUS 5

#define LED A8


CCU_status ccu_status;
Charger_configure charger_configure;
Charger_data charger_data;

BMS_status bms_status;
BMS_voltages bms_voltages;

BMS_detailed_voltages bms_detailed_voltages[8][4];
BMS_temperatures bms_temperatures;
BMS_detailed_temperatures bms_detailed_temperatures[8][2];
BMS_onboard_detailed_temperatures bms_onboard_detailed_temperatures[TOTAL_IC];
BMS_onboard_temperatures bms_onboard_temperatures;
BMS_balancing_status bms_balancing_status[(TOTAL_IC + 3) / 4]; // Round up TOTAL_IC / 4 since data from 4 ICs can fit in a single message

int watchdog_state = 0;
bool charge_enable = false;

static CAN_message_t rx_msg;
static CAN_message_t tx_msg;
FlexCAN CAN(500000);

Metro update_ls = Metro(1000);
Metro update_CAN = Metro(100);
Metro update_watchdog = Metro(3);

void print_cells();
void print_temps();
void parse_can_message();
void check_shutdown_signals();
void configure_charging();

void print_cells();
void print_temps();
void print_charger_data();

void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);

    pinMode(SHUTDOWN_B, INPUT);
    pinMode(SHUTDOWN_C, INPUT);
    pinMode(SHUTDOWN_D, INPUT);

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

    ccu_status.set_charger_enabled(true);
}

void loop() {
    
    if (update_CAN.check()) {
        ccu_status.write(tx_msg.buf);
        tx_msg.id = ID_CCU_STATUS;
        tx_msg.len = sizeof(ccu_status);
        CAN.write(tx_msg);

        tx_msg.ext = 1;
        charger_configure.write(tx_msg.buf);
        tx_msg.id = ID_CHARGER_CONTROL;
        tx_msg.len = sizeof(charger_configure);
        CAN.write(tx_msg);
        tx_msg.ext = 0;
    }
    
    if (update_watchdog.check()) {
        watchdog_state = !watchdog_state;
        digitalWrite(WATCHDOG_OUT, watchdog_state);
    }
    
    if (update_ls.check()) {
        print_cells();
        //print_temps();
        Serial.print("Charge enable: ");
        Serial.println(ccu_status.get_charger_enabled());
        Serial.print("BMS state: ");
        Serial.println(bms_status.get_state());
        print_charger_data();
    }

    check_shutdown_signals();
    
    
}

void parse_can_message() {
    while (CAN.read(rx_msg)) {
        if (rx_msg.id == ID_BMS_DETAILED_TEMPERATURES) {
            BMS_detailed_temperatures temp = BMS_detailed_temperatures(rx_msg.buf);
            bms_detailed_temperatures[temp.get_ic_id()][temp.get_group_id()].load(rx_msg.buf);
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
            charge_enable = ccu_status.get_charger_enabled();
        }

        if (rx_msg.id == ID_BMS_BALANCING_STATUS) {
            BMS_balancing_status temp = BMS_balancing_status(rx_msg.buf);
            bms_balancing_status[temp.get_group_id()].load(rx_msg.buf);
        }

        rx_msg.ext = 1;
        if (rx_msg.id == ID_CHARGER_DATA) {
            charger_data.load(rx_msg.buf);
        }
        rx_msg.ext = 0;
    }
}

void check_shutdown_signals() {
    int shutdown_b = digitalRead(SHUTDOWN_B);
    int shutdown_c = digitalRead(SHUTDOWN_C);
    int shutdown_d = digitalRead(SHUTDOWN_D);
    int shutdown_e = digitalRead(SHUTDOWN_E);
    int shutdown_f = digitalRead(SHUTDOWN_F);

    //if (shutdown_b == 0 || shutdown_c == 0 || shutdown_d == 0 || shutdown_e == 0) {
      //   ccu_status.set_charger_enabled(false);
    //
}

void configure_charging() {
    if (charge_enable) {
        charger_configure.set_max_charging_voltage_high(35);
        charger_configure.set_max_charging_voltage_low(0);
        charger_configure.set_max_charging_current_low(MAX_CURRENT * 10);
        charger_configure.set_control(0);
    } else {
        charger_configure.set_max_charging_voltage_high(0);
        charger_configure.set_max_charging_voltage_low(0);
        charger_configure.set_max_charging_current_low(0);
        charger_configure.set_control(1);
    }
}

void print_cells() {
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\t\t\t\tRaw Cell Voltages\t\t\t\t\t\t\tCell Status (Ignoring or Balancing)");
    Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8\t\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_EVEN_IC; cell++) {
            double voltage = bms_detailed_voltages[ic][cell / 3].get_voltage(cell % 3) * 0.0001;
            Serial.print(voltage, 4); Serial.print("V\t");
        }
        Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_EVEN_IC; cell++) {
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
        for (int cell = 0; cell < CELLS_PER_EVEN_IC; cell++) {
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
            double temp = ((double) bms_detailed_temperatures[ic][therm / 2].get_temperature(therm % 3)) / 100;
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

void print_charger_data(){
    uint8_t ac_voltage_high = charger_data.get_input_ac_voltage_high();
    uint8_t ac_voltage_low = charger_data.get_input_ac_voltage_low();
    uint8_t output_voltage_high = charger_data.get_output_dc_voltage_high();
    uint8_t output_voltage_low = charger_data.get_output_dc_voltage_low();
    uint8_t output_current_high = charger_data.get_output_current_high();
    uint8_t output_current_low = charger_data.get_output_current_low();


    
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.print(ac_voltage_high*16*16 + ac_voltage_low);
    Serial.print(" V\t\t");
    Serial.print(output_voltage_high*16*16 + output_voltage_low);
    Serial.print(" V\t\t");
    Serial.print(output_current_high*16*16 + output_current_low);
    Serial.print(" A\t\t");
    Serial.print(charger_data.get_flags());
    Serial.println();
}
