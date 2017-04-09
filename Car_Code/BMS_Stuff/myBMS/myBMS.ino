/**
 * Shrivathsav Seshan
 * Oct 23 2016
 * The BMS
 * Modified
 */

//#include <Arduino.h>
//#include <stdint.h>
//#include "Linduino.h"
//#include "LT_SPI.h"
//#include "LTC68041.h"
//#include "HyTech17.h"
//#include <SPI.h>
//#include <FlexCAN.h>
//#include <kinetis_flexcan.h>

#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "LTC68041.h"
#include "HyTech17.h"
#include <SPI.h>

/************BATTERY CONSTRAINTS AND CONSTANTS**********************/
#define VOLTAGE_LOW_CUTOFF 3000
#define VOLTAGE_HIGH_CUTOFF 4000
#define DISCHARGE_CURRENT_CONSTANT_HIGH 220
#define DISCHARGE_CURRENT_PEAK_HIGH 440
#define DISCHARGE_CURRENT_PEAK_HIGH_TIME 5
#define DISCHARGE_CURRENT_CONSTANT_HIGH_TIME 10
#define CHARGE_CURRENT_CONSTANT_HIGH -220
#define CHARGE_CURRENT_PEAK_HIGH -330
#define CHARGE_CURRENT_LOW_CUTOFF -1.1
#define CHARGE_CURRENT_PEAK_HIGH_TIME 10
#define CHARGE_CURRENT_CONSTANT_HIGH_TIME 20
#define CHARGE_TEMP_CRITICAL_HIGH 44
#define CHARGE_TEMP_CRITICAL_LOW 0
#define DISCHARGE_TEMP_CRITICAL_HIGH 60
#define DISCHARGE_TEMP_CRITICAL_LOW 15

/********GLOBAL ARRAYS/VARIABLES CONTAINING DATA FROM CHIP**********/
const uint8_t TOTAL_IC = 1;
uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages. Stores numbers in 0.1 mV units.
uint16_t aux_voltages[TOTAL_IC][6]; // contains auxiliary pin voltages.
                                     /* Data contained in this array is in this format:
                                      * Thermistor 1
                                      * Thermistor 2
                                      * Thermistor 3
                                      * Current Sensor
                                      */
int16_t cell_delta_voltage[TOTAL_IC][12]; // contains 12 signed dV values in 0.1 mV units

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
//FlexCAN can(500000);
//static CAN_message_t msg;
long msTimer = 0;

/**
 * BMS State Variables
 */
BMS_voltages bmsVoltageMessage;
BMS_currents bmsCurrentMessage;
BMS_temperatures bmsTempMessage;
BMS_status bmsStatusMessage;
int minVoltageICIndex;
int minVoltageCellIndex;
bool dischargeCurrentPeakHighFlag;
unsigned long dischargeCurrentPeakHighTime;
bool dischargeCurrentConstantHighFlag;
unsigned long dischargeCurrentConstantHighTime;
bool chargeCurrentPeakHighFlag;
unsigned long chargeCurrentPeakHighTime;
bool chargeCurrentConstantHighFlag;
unsigned long chargeCurrentConstantHighTime;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(2000);
    // SPI.begin();

    LTC6804_initialize();
    init_cfg();
//    can.begin();
    pollVoltage();
    memcpy(cell_delta_voltage, cell_voltages, 2 * TOTAL_IC * 12);
    Serial.println("Setup Complete!");
}

/*
 * Continuously poll voltages and print them to the Serial Monitor.
 */
 // NOTE: Implement Coulomb counting to track state of charge of battery.
void loop() {
    // put your main code here, to run repeatedly:
//    waitForUserInput();
    pollVoltage(); // cell_voltages[] array populated with cell voltages now.

//    pollAuxiliaryVoltages();
//    int thermValue = analogRead(A0);
//    Serial.print("Thermistor reading: "); Serial.println(thermValue);

    avgMinMaxTotalVoltage(); // min, max, avg, and total volts stored in bmsVoltageMessage object.
}

/*!***********************************
 \brief Initializes the configuration array
 **************************************/
void init_cfg()
{
    for(int i = 0; i < TOTAL_IC; i++)
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
        Serial.println("A PEC error was detected in cell voltage data");
    }
    printCells(); // prints the cell voltages to Serial.
    delay(100);
}

void pollAuxiliaryVoltages() {
    wakeFromSleepAllChips();
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
    wakeFromIdleAllChips();
    LTC6804_adax();
    delay(10);
    wakeup_idle();
    uint8_t error = LTC6804_rdaux(0, TOTAL_IC, aux_voltages);
    if (error == -1) {
        Serial.println("A PEC error was detected in auxiliary voltage data");
    }
    printAux();
    delay(100);
    // TODO: Take auxiliary voltage data from thermistor and convert raw analog data into temperature values.
    // TODO: Take auxilliary voltage data from current sensor and convert raw analog data into current values.
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
    uint16_t maxVolt = cell_voltages[0][0]; // stored in 0.1 mV units
    uint16_t minVolt = cell_voltages[0][0]; // stored in 0.1 mV units
    double avgVolt = 0; // stored as double volts
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < 12; cell++) {
            uint16_t currentCell = cell_voltages[ic][cell];
            cell_delta_voltage[ic][cell] = currentCell - cell_delta_voltage[ic][cell];
            if (currentCell > maxVolt) {
                maxVolt = currentCell;
            }
            if (currentCell < minVolt) {
                minVolt = currentCell;
                minVoltageICIndex = ic;
                minVoltageCellIndex = cell;
            }
            totalVolts += currentCell * 0.0001;
        }
    }
    avgVolt = totalVolts / (TOTAL_IC * 12); // stored as double volts
    bmsVoltageMessage.setAverage(static_cast<uint16_t>(avgVolt * 1000 + 0.5));
    bmsVoltageMessage.setTotal(static_cast<uint16_t>(totalVolts + 0.5));
    minVolt = convertToMillivolts(minVolt);
    maxVolt = convertToMillivolts(maxVolt);
    bmsVoltageMessage.setLow(minVolt);
    bmsVoltageMessage.setHigh(maxVolt);

    Serial.print("Avg: "); Serial.println(avgVolt, 4);
    Serial.print("Total: "); Serial.println(totalVolts, 4);
    Serial.print("Min: "); Serial.println(minVolt);
    Serial.print("Max: "); Serial.println(maxVolt);
}

void balanceCellsDuringCharging() {
    // 1 volt operating window. Balancing will kick in when any cell is greater than 0.25 volts of the lowest cell.
    uint16_t minVolt = bmsVoltageMessage.getLow(); // stored in mV
    int16_t minVoltDeltaVoltage = cell_delta_voltage[minVoltageICIndex][minVoltageCellIndex]; // stored in 0.1 mV
    double minTimeFactor = (4000.0 - minVolt) / (cell_delta_voltage[minVoltageICIndex][minVoltageCellIndex] * 0.1);
    uint8_t batteryIndexCounter = 0;
    uint8_t* batteryBalanceMask = (uint8_t*) malloc(sizeof(uint8_t) * 12 * TOTAL_IC);
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < 12; cell++) {
            double currentTimeFactor = (4000.0 - cell_voltages[ic][cell] * 0.1) / (cell_delta_voltage[ic][cell] * 0.1);
            uint8_t val = 0b00000001;
            if (currentTimeFactor < minTimeFactor) {
                // when the current cell is charging FASTER than the lowest voltage cell in the pack
                // turn on balancing, a.k.a discharge this cell indefinitely.
                if (cell < 8) {
                    val = val << cell;
                    tx_cfg[ic][4] = tx_cfg[ic][4] | val;
                } else {
                    val = 0b00000001;
                    val = val << (cell - 8);
                    tx_cfg[ic][5] = tx_cfg[ic][5] | val;
                }
                tx_cfg[ic][5] = tx_cfg[ic][5] | 0b11110000;
            } else if (currentTimeFactor > minTimeFactor) {
                // when the current cell is charging slower than the lowest voltage cell in the pack
                // turn off balancing, a.k.a STOP discharging this cell.
                uint8_t mask = 0b11111111;
                if (cell < 8) {
                    val = 0b00000001;
                    val = val << cell;
                    val = val ^ mask;
                    tx_cfg[ic][4] = tx_cfg[ic][4] & val;
                } else {
                    val = 0b00000001;
                    val = val << (cell - 8);
                    val = val ^ mask;
                    tx_cfg[ic][5] = tx_cfg[ic][5] & val;
                }
            }
        }
    }
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}

void raiseVoltageTempCurrentFlags() {
    float current = bmsCurrentMessage.getCurrent(); // stored in amps
    if (current < 0) {
        // when batteries are charging
        if (bmsVoltageMessage.getLow() < VOLTAGE_LOW_CUTOFF) {
            bmsStatusMessage.setChargeUndervoltage(true);
            bmsStatusMessage.setBMSStatusOK(false);
        }

        if (bmsVoltageMessage.getHigh() > VOLTAGE_HIGH_CUTOFF) {
            bmsStatusMessage.setChargeOvervoltage(true);
            bmsStatusMessage.setBMSStatusOK(false);
        }

        if (current < CHARGE_CURRENT_CONSTANT_HIGH) {
            if (chargeCurrentConstantHighFlag) {
                if (millis() - chargeCurrentConstantHighTime > CHARGE_CURRENT_CONSTANT_HIGH_TIME) {
                    // constant charging current flow has exceeded limit and time
                    bmsStatusMessage.setChargeOvercurrent(true);
                    bmsStatusMessage.setBMSStatusOK(false);
                }
            } else {
                chargeCurrentConstantHighFlag = true;
                chargeCurrentConstantHighTime = millis();
            }
        } else {
            chargeCurrentConstantHighFlag = false;
        }

        if (current < CHARGE_CURRENT_PEAK_HIGH) {
            if (chargeCurrentPeakHighFlag) {
                if (millis() - chargeCurrentPeakHighTime > CHARGE_CURRENT_PEAK_HIGH_TIME) {
                    // peak charging current flow has exceeded limit and time
                    bmsStatusMessage.setChargeOvercurrent(true);
                    bmsStatusMessage.setBMSStatusOK(false);
                }
            } else {
                chargeCurrentPeakHighFlag = true;
                chargeCurrentConstantHighTime = millis();
            }
        } else {
            chargeCurrentPeakHighFlag = false;
        }

        if (current > CHARGE_CURRENT_LOW_CUTOFF) {
            bmsStatusMessage.setChargeUndercurrent(true);
            bmsStatusMessage.setBMSStatusOK(false);
        }
        if (bmsTempMessage.getHighTemp() > CHARGE_TEMP_CRITICAL_HIGH) {
            bmsStatusMessage.setChargeOvertemp(true);
            bmsStatusMessage.setBMSStatusOK(false);
        }
    } else if (current > 0) {
        // when batteries are discharging
        if (bmsVoltageMessage.getLow() < VOLTAGE_LOW_CUTOFF) {
            bmsStatusMessage.setDischargeUndervoltage(true);
            bmsStatusMessage.setBMSStatusOK(false);
        }

        if (bmsVoltageMessage.getHigh() > VOLTAGE_HIGH_CUTOFF) {
            bmsStatusMessage.setDischargeOvervoltage(true);
            bmsStatusMessage.setBMSStatusOK(false);
        }

        if (current > DISCHARGE_CURRENT_CONSTANT_HIGH) {
            if (dischargeCurrentConstantHighFlag) {
                if (millis() - dischargeCurrentConstantHighTime > DISCHARGE_CURRENT_CONSTANT_HIGH_TIME) {
                    // constant discharging current draw has exceeded time and limit
                    bmsStatusMessage.setDischargeOvercurrent(true);
                    bmsStatusMessage.setBMSStatusOK(false);
                }
            } else {
                dischargeCurrentConstantHighFlag = true;
                dischargeCurrentConstantHighTime = millis();
            }
        } else {
            dischargeCurrentConstantHighFlag = false;
        }

        if (current > DISCHARGE_CURRENT_PEAK_HIGH) {
            if (dischargeCurrentPeakHighFlag) {
                if (millis() - dischargeCurrentPeakHighTime > DISCHARGE_CURRENT_PEAK_HIGH_TIME) {
                    // peak discharging current draw has exceeded time and limit
                    bmsStatusMessage.setDischargeOvercurrent(true);
                    bmsStatusMessage.setBMSStatusOK(false);
                }
            } else {
                dischargeCurrentPeakHighFlag = true;
                dischargeCurrentPeakHighTime = millis();
            }
        } else {
            dischargeCurrentPeakHighFlag = false;
        }

        if (bmsTempMessage.getHighTemp() > CHARGE_TEMP_CRITICAL_HIGH) {
            bmsStatusMessage.setDischargeOvertemp(true);
            bmsStatusMessage.setBMSStatusOK(false);
        }
    }
}

uint16_t convertToMillivolts(uint16_t v) {
    return (v + 5) / 10;
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

void printAux() {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
        Serial.print("IC: ");
        Serial.println(current_ic + 1);
        for (int i = 0; i < 6; i++) {
            Serial.print("Aux-"); Serial.print(i+1); Serial.print(": ");
            float voltage = aux_voltages[current_ic][i] * 0.0001;
            Serial.println(voltage, 4);
        }
        Serial.println();
    }
}

void waitForUserInput() {
    if (Serial.available()) {
        while (Serial.available()) {
            Serial.read();
        }
    } // clear buffer
    Serial.println("Continue?");
    while (!Serial.available()) {
        // wait for input
    }
    Serial.println();
}
