/**
 * LinduinoBMS.ino - Code that runs on the BMS Linduino. This system only has one Linduino, which interfaces directly with the Car's CAN Bus
 * Created by Shrivathsav Seshan & Charith "Karvin" Dassanayake, April 11, 2017
 */

#include <Arduino.h>
#include <math.h>
#include "mcp_can.h"
#include "LTC68041.h"
#include "HyTech17.h"

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
#define TOTAL_IC 9
#define TOTAL_CELLS 12
#define TOTAL_THERMISTORS 4
#define THERMISTOR_RESISTOR_VALUE 1000
#define AUX_VOLTAGE_CURRENT_INDEX 5
uint16_t cell_voltages[TOTAL_IC][TOTAL_CELLS]; // contains 12 battery cell voltages. Stores numbers in 0.1 mV units.
uint16_t aux_voltages[TOTAL_IC][6]; // contains auxiliary pin voltages.
     /* Data contained in this array is in this format:
      * Thermistor 1
      * Thermistor 2
      * Thermistor 3
      * Thermistor 4
      * Current Sensor
      */
int16_t cell_delta_voltage[TOTAL_IC][TOTAL_CELLS]; // contains 12 signed dV values in 0.1 mV units

/*!<
  The tx_cfg[][6] stores the LTC6804 configuration data that is going to be written
  to the LTC6804 ICs on the daisy chain. The LTC6804 configuration data that will be
  written should be stored in blocks of 6 bytes. The array should have the following format:

 |  tx_cfg[0][0]| tx_cfg[0][1] |  tx_cfg[0][2]|  tx_cfg[0][3]|  tx_cfg[0][4]|  tx_cfg[0][5]| tx_cfg[1][0] |  tx_cfg[1][1]|  tx_cfg[1][2]|  .....    |
 |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
 |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |

*/
uint8_t tx_cfg[TOTAL_IC][6]; // data defining how data will be written to daisy chain ICs.

/**
 * CAN Variables
 */
#define CAN_SPI_CS_PIN 5
MCP_CAN CAN(CAN_SPI_CS_PIN);
long msTimer = 0;

/**
 * BMS State Variables
 */
#define BMS_OK_PIN 5
BMS_voltages bmsVoltageMessage;
BMS_currents bmsCurrentMessage;
BMS_temperatures bmsTempMessage;
BMS_status bmsStatusMessage;

int minVoltageICIndex;
int minVoltageCellIndex;

//bool dischargeCurrentPeakHighFlag;
//unsigned long dischargeCurrentPeakHighTime;
//bool dischargeCurrentConstantHighFlag;
//unsigned long dischargeCurrentConstantHighTime;
//bool chargeCurrentPeakHighFlag;
//unsigned long chargeCurrentPeakHighTime;
//bool chargeCurrentConstantHighFlag;
//unsigned long chargeCurrentConstantHighTime;

void setup() {
    // put your setup code here, to run once:
    pinMode(BMS_OK_PIN, OUTPUT);
    // pinMode(CAN_SPI_CS_PIN, OUTPUT); Not needed, done in mcp_can.cpp

    digitalWrite(CAN_SPI_CS_PIN, HIGH);
    digitalWrite(BMS_OK_PIN, HIGH);

    Serial.begin(115200);
    delay(2000);

    // Check CAN Initialization
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {
        Serial.println("Init CAN BUS Shield FAILED. Retrying");
        delay(100);
    }
    Serial.println("CAN BUS Shield init GOOD");

    LTC6804_initialize();
    init_cfg();
    pollVoltage();
    memcpy(cell_delta_voltage, cell_voltages, 2 * TOTAL_IC * TOTAL_CELLS);
    bmsCurrentMessage.setChargingState(CHARGING);
    Serial.println("Setup Complete!");
}

// NOTE: Implement Coulomb counting to track state of charge of battery.
/*
 * Main BMS Control Loop
 */
void loop() {
//    waitForUserInput();
    pollVoltage(); // cell_voltages[] array populated with cell voltages now.
    balanceCellsDuringCharging();

    pollAuxiliaryVoltages();
    avgMinMaxTotalVoltage(); // min, max, avg, and total volts stored in bmsVoltageMessage object.
    raiseVoltageFlags();

    // write to CAN!
    writeToCAN();

    // set BMS_OK signal
    if (!bmsStatusMessage.getBMSStatusOK()) {
        digitalWrite(BMS_OK_PIN, LOW);
    }
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
//    dischargeAll();
}

void dischargeAll() {
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][4] = 0b11111111;
        tx_cfg[i][5] = tx_cfg[i][5] | 0b00001111;
    }
    wakeFromSleepAllChips();
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
}

void avgLowHighTemp() {
    uint16_t avgTemp, lowTemp, highTemp, totalTemp;
    totalTemp = 0;
    lowTemp = calculateDegreesCelsius(calculateThermistorResistance(aux_voltages[0][0]));
    highTemp = lowTemp;
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int therm = 0; therm < TOTAL_THERMISTORS; therm++) {
            uint16_t thermTemp = calculateDegreesCelsius(calculateThermistorResistance(aux_voltages[ic][therm]));
            if (thermTemp < lowTemp) {
                lowTemp = thermTemp;
            }
            if (thermTemp > highTemp) {
                highTemp = thermTemp;
            }
            totalTemp += thermTemp;
        }
    }
    avgTemp = totalTemp / (TOTAL_IC * TOTAL_THERMISTORS);
    bmsTempMessage.setLowTemp(lowTemp);
    bmsTempMessage.setHighTemp(highTemp);
    bmsTempMessage.setAvgTemp(avgTemp);

    Serial.print("Low Temp: ");
    Serial.print(lowTemp / 100); Serial.print("."); Serial.println(lowTemp % 100);
    Serial.print("High Temp: ");
    Serial.print(highTemp / 100); Serial.print("."); Serial.println(highTemp % 100);
    Serial.print("Average Temp: ");
    Serial.print(avgTemp / 100); Serial.print("."); Serial.println(avgTemp % 100);
}

uint16_t calculateThermistorResistance(uint16_t tempVoltage) {
    // voltage measured across thermistor is dependent on the resistor in the voltage divider
    // v = 5 (Rt / (Rt + R1));
    // Rt*v + R1*v = 5*Rt;
    // Rt*v - 5*Rt = -R1*v;
    // Rt = (R1*v) / (5 - v);
    // all voltage measurements stored in arrays are in 0.1 mV, or 1/10,000 of a volt
    return (THERMISTOR_RESISTOR_VALUE * tempVoltage) / (50000 - tempVoltage);
    // resistances stored as 1 ohm units.
}

uint16_t calculateDegreesCelsius(uint16_t thermistorResistance) {
    // temperature equation based on resistance is the following
    // R_inf = R0 * e^(-B / T0);
    // T = B / ln((R/R0) * e^(B / T0))
    // T = B / (ln(R/R0) + ln(e^(B / T0)))
    // T = B / (ln(R/R0) + (B / T0))
    // B = 3984
    // R0 = 10000
    double temp = 3984 / (log((1.0 * thermistorResistance) / 1.0e5) + (3984.0 / 298.15));
    return (int) ((temp - 273.15) * 100);
    // temps stored in 0.01 C units
}

void calculateCurrent() {
    // TODO: Take auxiliary voltage data from current sensor and convert raw analog data into current values.
    
}

void wakeFromSleepAllChips() {
    for (int i = 0; i < TOTAL_IC / 3; i++) {
        wakeup_sleep();
//        delay(3);
    }
}

void wakeFromIdleAllChips() {
    for (int i = 0; i < TOTAL_IC / 3; i++) {
        wakeup_idle();
//        delay(3);
    }
}

void avgMinMaxTotalVoltage() {
    double totalVolts = 0; // stored as double volts
    uint16_t maxVolt = cell_voltages[0][0]; // stored in 0.1 mV units
    uint16_t minVolt = cell_voltages[0][0]; // stored in 0.1 mV units
    double avgVolt = 0; // stored as double volts
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < TOTAL_CELLS; cell++) {
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
    avgVolt = totalVolts / (TOTAL_IC * TOTAL_CELLS); // stored as double volts
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
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < TOTAL_CELLS; cell++) {
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
    wakeFromSleepAllChips();
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
}

void raiseVoltageFlags() {
    if (bmsVoltageMessage.getLow() <= VOLTAGE_LOW_CUTOFF) {
        bmsStatusMessage.setChargeUndervoltage(true);
        bmsStatusMessage.setDischargeUndervoltage(true);
        bmsStatusMessage.setBMSStatusOK(false);
    } else if (bmsVoltageMessage.getHigh() >= VOLTAGE_HIGH_CUTOFF) {
        bmsStatusMessage.setChargeOvervoltage(true);
        bmsStatusMessage.setDischargeOvervoltage(true);
        bmsStatusMessage.setBMSStatusOK(false);
    } else {
        bmsStatusMessage.clearAllFlags();
        bmsStatusMessage.setBMSStatusOK(true);
    }
}

void writeToCAN() {
    digitalWrite(10, HIGH);
    unsigned char msg[8] = {0,0,0,0,0,0,0,0};
    bmsVoltageMessage.write(msg);
    byte CANsendMsgResult = CAN.sendMsgBuf(ID_BMS_VOLTAGE, 0, 8, msg);
    // if (CANsendMsgResult == CAN_OK) {
    //     Serial.println("CAN bms voltage message sent");
    // } else if (CANsendMsgResult == CAN_GETTXBFTIMEOUT) {
    //     Serial.println("CAN bms voltage get tx buffer timeout");
    // } else if (CANsendMsgResult == CAN_SENDMSGTIMEOUT) {
    //     Serial.println("CAN bms voltage send message timeout");
    // } else {
    //     Serial.println("CAN bms voltage error unknown");
    // }

    bmsCurrentMessage.setChargingState(CHARGING);
    bmsCurrentMessage.write(msg);
    if (bmsCurrentMessage.getChargingState() == CHARGING) {
        Serial.println("CHARGING");
    } else if (bmsCurrentMessage.getChargingState() == DISCHARGING) {
        Serial.println("DISCHARGING");
    }
    CANsendMsgResult = CAN.sendMsgBuf(ID_BMS_CURRENT, 0, 8, msg);
    // if (CANsendMsgResult == CAN_OK) {
    //     Serial.println("CAN bms current message sent");
    // } else if (CANsendMsgResult == CAN_GETTXBFTIMEOUT) {
    //     Serial.println("CAN bms current get tx buffer timeout");
    // } else if (CANsendMsgResult == CAN_SENDMSGTIMEOUT) {
    //     Serial.println("CAN bms current send message timeout");
    // } else {
    //     Serial.println("CAN bms current error unknown");
    // }

    bmsTempMessage.write(msg);
    CANsendMsgResult = CAN.sendMsgBuf(ID_BMS_TEMPERATURE, 0, 8, msg);
    // if (CANsendMsgResult == CAN_OK) {
    //     Serial.println("CAN bms temperature message sent");
    // } else if (CANsendMsgResult == CAN_GETTXBFTIMEOUT) {
    //     Serial.println("CAN bms temperature get tx buffer timeout");
    // } else if (CANsendMsgResult == CAN_SENDMSGTIMEOUT) {
    //     Serial.println("CAN bms temperature send message timeout");
    // } else {
    //     Serial.println("CAN bms temperature error unknown");
    // }

    bmsStatusMessage.write(msg);
    CANsendMsgResult = CAN.sendMsgBuf(ID_BMS_STATUS, 0, 8, msg);
    // if (CANsendMsgResult == CAN_OK) {
    //     Serial.println("CAN bms stats message sent");
    // } else if (CANsendMsgResult == CAN_GETTXBFTIMEOUT) {
    //     Serial.println("CAN bms status get tx buffer timeout");
    // } else if (CANsendMsgResult == CAN_SENDMSGTIMEOUT) {
    //     Serial.println("CAN bms status send message timeout");
    // } else {
    //     Serial.println("CAN bms status error unknown");
    // }
    digitalWrite(10, HIGH);
}

uint16_t convertToMillivolts(uint16_t v) {
    return (v + 5) / 10;
}

void printCells() {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
        Serial.print("IC: ");
        Serial.println(current_ic+1);
        for (int i = 0; i < TOTAL_CELLS; i++) {
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
