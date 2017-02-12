/**
 * Shrivathsav Seshan
 * Oct 23 2016
 * The BMS
 * Modified
 */

#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "LTC68041.h"
#include "HyTech17.h"
#include <SPI.h>
#include <FlexCAN.h>
#include <kinetis_flexcan.h>

/************BATTERY CONSTRAINTS AND CONSTANTS**********************/
#define VOLTAGE_LOW_CUTOFF 3.0
#define VOLTAGE_HIGH_CUTOFF 4.2
#define DISCHARGE_CURRENT_CONSTANT_HIGH 220
#define DISCHARGE_CURRENT_PEAK_HIGH 440
#define CHARGE_CURRENT_CONSTANT_HIGH 220
#define CHARGE_CURRENT_PEAK_HIGH 330
#define CHARGE_CURRENT_LOW_CUTOFF 1.1
#define CHARGE_TEMP_CRITICAL_HIGH 44
#define CHARGE_TEMP_CRITICAL_LOW 0
#define DISCHARGE_TEMP_CRITICAL_HIGH 60
#define DISCHARGE_TEMP_CRITICAL_LOW 15
#define MAX_16BIT_UNSIGNED 65536

/********GLOBAL ARRAYS/VARIABLES CONTAINING DATA FROM CHIP**********/
const uint8_t TOTAL_IC = 1;
uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages. Stores numbers in 0.1 mV.
uint16_t aux_voltagess[TOTAL_IC][6]; // contains auxillary pin voltages (not batteries)

/*!<
  The tx_cfg[][6] stores the LTC6804 configuration data that is going to be written
  to the LTC6804 ICs on the daisy chain. The LTC6804 configuration data that will be
  written should be stored in blocks of 6 bytes. The array should have the following format:

 |  tx_cfg[0][0]| tx_cfg[0][1] |  tx_cfg[0][2]|  tx_cfg[0][3]|  tx_cfg[0][4]|  tx_cfg[0][5]| tx_cfg[1][0] |  tx_cfg[1][1]|  tx_cfg[1][2]|  .....    |
 |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
 |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |

*/
uint8_t tx_cfg[TOTAL_IC][6]; // data defining how data will be written to daisy chain ICs.

/*!<
  the rx_cfg[][8] array stores the data that is read back from a LTC6804-1 daisy chain.
  The configuration data for each IC  is stored in blocks of 8 bytes. Below is an table illustrating the array organization:

|rx_config[0][0]|rx_config[0][1]|rx_config[0][2]|rx_config[0][3]|rx_config[0][4]|rx_config[0][5]|rx_config[0][6]  |rx_config[0][7] |rx_config[1][0]|rx_config[1][1]|  .....    |
|---------------|---------------|---------------|---------------|---------------|---------------|-----------------|----------------|---------------|---------------|-----------|
|IC1 CFGR0      |IC1 CFGR1      |IC1 CFGR2      |IC1 CFGR3      |IC1 CFGR4      |IC1 CFGR5      |IC1 PEC High     |IC1 PEC Low     |IC2 CFGR0      |IC2 CFGR1      |  .....    |
*/
uint8_t rx_cfg[TOTAL_IC][8];

/**
 * CAN Variables
 */
FlexCAN can(500000);
static CAN_message_t msg;
long msTimer = 0;

/**
 * BMS State Variables
 */
BMS_voltages bmsVoltageMessage;
BMS_currents bmsCurrentMessage;
BMS_temperatures bmsTempMessage;
// TODO: Add BMS Status to CAN Library. To send fault messages.
boolean BMSStatusOK;
boolean tractiveSystemOn;

/**
 * Teensy Communication Pin Constants
 */
#define MOSI_PIN 17
#define MISO_PIN 16
#define SPI_CHIP_SELECT_PIN 15
#define CLOCK_PIN 14
#define T_WAKE 50

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    SPI.setMOSI(MOSI_PIN);
    SPI.setMISO(MISO_PIN);
    SPI.setSCK(CLOCK_PIN);
    pinMode(SPI_CHIP_SELECT_PIN, OUTPUT);
    SPI.begin();

    LTC6804_initialize();
    init_cfg();
    can.begin();
    BMSStatusOK = true;
}

/*
 * Continuously poll voltages and print them to the Serial Monitor.
 */
 // NOTE: Implement Coulomb counting to track state of charge of battery.
void loop() {
    // put your main code here, to run repeatedly:
    pollVoltage(); // cell_voltages[] array populated with cell voltages now.
    printCells();

    pollThermistors();
    pollCurrent();

    avgMinMaxTotalVoltage(); // min, max, avg, and total volts stored in bmsVoltageMessage object.
}

/*!***********************************
 \brief Initializes the configuration array
 **************************************/
void init_cfg()
{
    // TODO: Write Threshold Values into Configuration Registers
    for(int i = 0; i<TOTAL_IC;i++)
    {
        tx_cfg[i][0] = 0xFE;
        tx_cfg[i][1] = 0x00 ;
        tx_cfg[i][2] = 0x00 ;
        tx_cfg[i][3] = 0x00 ;
        tx_cfg[i][4] = 0x00 ;
        tx_cfg[i][5] = 0x00 ;
    }
}

void pollVoltage() {
    Serial.println("Polling Voltages...");
    /*
     * Difference between wakeup_sleep and wakeup_idle
     * wakeup_sleep wakes up the LTC6804 from sleep state
     * wakeup_idle wakes up the isoSPI port.
     */
    wakeFromSleepAllChips();
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
    wakeFromIdleAllChips();
    LTC6804_adcv();
    delay(10);
    wakeup_idle();
    uint8_t error = LTC6804_rdcv(0, TOTAL_IC, cell_voltages); // asks chip to read voltages and stores in given array.
    if (error == -1) {
        Serial.println("A PEC error was detected in voltage data");
    }
    printCells(); // prints the cell voltages to Serial.
    delay(100);
}

void pollThermistors() {
    // TODO: Implement function to poll auxillary registers that are connected to thermistors
}

void pollCurrent() {
    // TODO: Implement function to poll auxillary registers that are connected to current sensor.
}

void wakeFromSleepAllChips() {
    for (int i = 0; i < TOTAL_IC / 3; i++) {
        wakeup_sleep();
        delay(3);
    }
}

void wakeFromIdleAllChips() {
    for (int i = 0; i < TOTAL_IC / 3; i++) {
        wakeup_idle();
        delay(3);
    }
}

void avgMinMaxTotalVoltage() {
    double totalVolts = 0; // stored as double volts
    uint16_t maxVolt = cell_voltages[0][0];
    uint16_t minVolt = cell_voltages[0][0];
    double avgVolt = 0; // stored as double volts
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < 12; cell++) {
            uint16_t currentCell = cell_voltages[ic][cell];
            if (currentCell > maxVolt) {
                maxVolt = currentCell;
            }
            if (currentCell < minVolt) {
                minVolt = currentCell;
            }
            totalVolts += currentCell * 0.0001;
        }
    }
    avgVolt = totalVolts / (TOTAL_IC * 12); // stored as double volts
    bmsVoltageMessage.avgVoltage = static_cast<uint16_t>(avgVolt * 1000 + 0.5);
    bmsVoltageMessage.totalVoltage = static_cast<uint16_t>(totalVolts + 0.5);
    bmsVoltageMessage.lowVoltage = minVolt;
    bmsVoltageMessage.highVoltage = maxVolt;
}



uint16_t convertToMillivolts(uint16_t v) {
    return v / 10;
}

void printCells() {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
        Serial.print("IC: ");
        Serial.println(current_ic+1);
        for (int i = 0; i < 12; i++) {
            Serial.print("C"); Serial.print(i+1); Serial.print(": ");
            float voltage = cell_voltages[current_ic][i] * 0.0001;
            Serial.println(voltage, 4);
        }
        Serial.println();
    }
}

void printCanMessage() {
    uint8_t len = msg.len;
    Serial.print("ID: ");
    Serial.println(msg.id);
    Serial.print("MESSAGE: ");
    for (uint8_t i = 0; i < len; i++) {
        Serial.print(msg.buf[i]);
    }
    Serial.println();
}
