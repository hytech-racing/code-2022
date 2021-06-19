/*
    Program to check voltage information of the battery pack.
    Upload this code to a Teensy 3.2 connected to the same CAN bus
    with the BMS, and it will print voltage information to Serial.

    Created March 2019
*/

#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>

#define TOTAL_IC 8                      // Number of ICs in the system
#define CELLS_PER_IC 9                  // Number of cells per IC
#define THERMISTORS_PER_IC 3            // Number of cell thermistors per IC
#define PCB_THERM_PER_IC 2              // Number of PCB thermistors per IC

BMS_status bms_status;
BMS_voltages bms_voltages;

BMS_detailed_voltages bms_detailed_voltages[8][3];
BMS_temperatures bms_temperatures;
BMS_detailed_temperatures bms_detailed_temperatures[8];
BMS_onboard_detailed_temperatures bms_onboard_detailed_temperatures[TOTAL_IC];
BMS_onboard_temperatures bms_onboard_temperatures;
uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages. Numbers are stored in 0.1 mV units.

static CAN_message_t rx_msg;
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
    parse_can_message();
    process_voltages();
    print_cells();
    print_temps();

    delay(500);
}

void process_voltages() {
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            cell_voltages[ic][cell] = bms_detailed_voltages[ic][cell / 3].get_voltage(cell % 3);
        }
    }
}

void print_cells() {
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
    Serial.println("\t\t\t\tRaw Cell Voltages\t\t\t\t\t\t\tCell Status (Ignoring or Balancing)");
    Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8\t\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            double voltage = cell_voltages[ic][cell] * 0.0001;
            Serial.print(voltage, 4); Serial.print("V\t");
        }
        Serial.println();
    }
    Serial.println();
    Serial.println("\t\t\t\tDelta from Min Cell");
    Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        Serial.print("IC"); Serial.print(ic); Serial.print("\t");
        for (int cell = 0; cell < CELLS_PER_IC; cell++) {
            double voltage = (cell_voltages[ic][cell]-bms_voltages.get_low()) * 0.0001;
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

        if (rx_msg.id == ID_BMS_VOLTAGES)
        {
            bms_voltages.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_TEMPERATURES)
        {
            bms_temperatures.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_ONBOARD_TEMPERATURES)
        {
            bms_onboard_temperatures.load(rx_msg.buf);
        }

        if (rx_msg.id == ID_BMS_ONBOARD_DETAILED_TEMPERATURES)
        {
            BMS_onboard_detailed_temperatures temp = BMS_onboard_detailed_temperatures(rx_msg.buf);
            bms_onboard_detailed_temperatures[temp.get_ic_id()].load(rx_msg.buf);
        }
    }
}
