/**
 * LinduinoBMS.ino - Code that runs on the BMS Linduino. This system only has one Linduino, which interfaces directly with the Car's CAN Bus
 * Created by Shrivathsav Seshan & Charith "Karvin" Dassanayake, April 11, 2017
 */

#include <Arduino.h>
#include <math.h>
#include <stdint.h>
#include "mcp_can.h"
#include "LTC68041.h"
#include "HyTech17.h"

/*
 * On startup.
 * 1. GLV boxes latches shutdown circuit closed.
 * 2. AIR's close.
 * 3. Voltage flows out of box, and turn on lights.
 * 4. Any faults (OK_HS, BMS_OK, BSPD) will open shutdown circuit, open AIR's.
 */

/*
 * Operating condition notes:
 * 1. BMS sensors can be powered at all times.
 * 2. Once linduino gets power from external power lines, give OK_BMS signal.
 * 3. No need to check DC bus voltage, because all batteries read their true voltages at all times. (They are continuous with each other at all times, due to no relay.)
 * 4. Once Temps go too high, current goes too high, or cell voltages go too high or too low, drive the BMS_OK signal low.
 */

/************BATTERY CONSTRAINTS AND CONSTANTS**********************/
#define VOLTAGE_LOW_CUTOFF 2980
#define VOLTAGE_HIGH_CUTOFF 4210
#define TOTAL_VOLTAGE_CUTOFF 150
#define DISCHARGE_CURRENT_CONSTANT_HIGH 220
#define DISCHARGE_CURRENT_PEAK_HIGH 440
#define DISCHARGE_CURRENT_PEAK_HIGH_TIME 5
#define DISCHARGE_CURRENT_CONSTANT_HIGH_TIME 10
#define CHARGE_CURRENT_CONSTANT_HIGH -400
#define CHARGE_CURRENT_PEAK_HIGH -400
#define CHARGE_CURRENT_PEAK_HIGH_TIME 10
#define CHARGE_CURRENT_CONSTANT_HIGH_TIME 20
#define MAX_VAL_CURRENT_SENSE 300
#define CHARGE_TEMP_CRITICAL_HIGH 4400 // 44.00
#define DISCHARGE_TEMP_CRITICAL_HIGH 6000 // 60.00

/********GLOBAL ARRAYS/VARIABLES CONTAINING DATA FROM CHIP**********/
#define TOTAL_IC 4
#define TOTAL_CELLS 9
#define TOTAL_THERMISTORS 3 // TODO: Double check how many thermistors are being used.
#define THERMISTOR_RESISTOR_VALUE 6700 // TODO: Double check what resistor is used on the resistor divider.
uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages. Stores numbers in 0.1 mV units.
uint16_t aux_voltages[TOTAL_IC][6]; // contains auxiliary pin voltages.
     /* Data contained in this array is in this format:
      * Thermistor 1
      * Thermistor 2
      * Thermistor 3
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

/**
 * CAN Variables
 */
#define CAN_SPI_CS_PIN 5
MCP_CAN CAN(CAN_SPI_CS_PIN);
long msTimer = 0;

/**
 * BMS State Variables
 */
#define BMS_OK_PIN 3
#define WATCH_DOG_TIMER 4
#define CURRENT_SENSE 0
bool watchDogFlag = true;
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
    pinMode(WATCH_DOG_TIMER, OUTPUT);
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
    //   balanceCellsDuringCharging();
    process_voltages(); // stores data in bmsVoltageMessage object.

    pollAuxiliaryVoltages();
    process_temps(); // stores datap in bmsTempMessage object.
    process_current(); // stores data in bmsCurrentMessage object.

    // write to CAN!
    writeToCAN();

    // set BMS_OK signal
    if (!bmsStatusMessage.getBMSStatusOK()) {
        digitalWrite(BMS_OK_PIN, LOW);
        Serial.println("STATUS NOT GOOD!!!!!!!!!!!!!!!");
    }
    watchDogFlag = !watchDogFlag; // inverting watchDogFlag
    // Prevents WATCH_DOG_TIMER timeout
    digitalWrite(WATCH_DOG_TIMER, watchDogFlag);
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
    wakeFromSleepAllChips();
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
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
//    printCells(); // prints the cell voltages to Serial.
    delay(200); // TODO: Why 200 milliseconds?
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
//    printAux();
    delay(200);
}

void process_temps() {
    double avgTemp, lowTemp, highTemp, totalTemp;
    totalTemp = 0;
    lowTemp = calculateDegreesCelsius(thermistorResistanceGPIO12(aux_voltages[0][0]));
    highTemp = lowTemp;
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        if (ic != 2) {
            Serial.println("Thermistor 1");
            uint16_t resistance = thermistorResistanceGPIO12(aux_voltages[ic][0]);
            uint16_t thermTemp = calculateDegreesCelsius(resistance);
            if (thermTemp < lowTemp) {
                lowTemp = thermTemp;
            }
            if (thermTemp > highTemp) {
                highTemp = thermTemp;
            }
            totalTemp += thermTemp;
            Serial.println("Thermistor 2");
            resistance = thermistorResistanceGPIO12(aux_voltages[ic][1]);
            thermTemp = calculateDegreesCelsius(resistance);
            if (thermTemp < lowTemp) {
                lowTemp = thermTemp;
            }
            if (thermTemp > highTemp) {
                highTemp = thermTemp;
            }
            totalTemp += thermTemp;
            Serial.println("Thermistor 3");
            resistance = thermistorResistanceGPIO12(aux_voltages[ic][2]);
            thermTemp = calculateDegreesCelsius(resistance);
            if (thermTemp < lowTemp) {
                lowTemp = thermTemp;
            }
            if (thermTemp > highTemp) {
                highTemp = thermTemp;
            }
            totalTemp += thermTemp;
        }
        Serial.println("----------------------\n");
    }
    avgTemp = (uint16_t) (totalTemp / ((3) * TOTAL_THERMISTORS));
    bmsTempMessage.setLowTemp((uint16_t) lowTemp);
    bmsTempMessage.setHighTemp((uint16_t) highTemp);
    bmsTempMessage.setAvgTemp((uint16_t) avgTemp);

    if (bmsCurrentMessage.getChargingState() == 0) { // discharging
        if (bmsTempMessage.getHighTemp() > DISCHARGE_TEMP_CRITICAL_HIGH) {
            bmsStatusMessage.setDischargeOvertemp(true);
            bmsStatusMessage.setBMSStatusOK(false);
            Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
        }
    } else if (bmsCurrentMessage.getChargingState() == 1) { // charging
        if (bmsTempMessage.getHighTemp() > CHARGE_TEMP_CRITICAL_HIGH) {
            bmsStatusMessage.setChargeOvertemp(true);
            bmsStatusMessage.setBMSStatusOK(false);
            Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
        }
    }

    Serial.print("Low Temp: ");
    Serial.println(lowTemp / 100);
    Serial.print("High Temp: ");
    Serial.println(highTemp / 100);
    Serial.print("Average Temp: ");
    Serial.println(avgTemp / 100);
}

uint16_t thermistorResistanceGPIO12(double tempVoltage) {
    /* voltage measured across thermistor is dependent on the resistor in the voltage divider
     * v = 5 - 5 (Rt / (Rt + R1));
     * Rt*v + R1*v = -5*Rt + 5*Rt + 5R1
     * Rt*v = 5*R1 - R1*v
     * Rt = (5*R1 - R1*v) / v
     * all voltage measurements stored in arrays are in 0.1 mV, or 1/10,000 of a volt
     */
     double resistance = (50000.0 - tempVoltage) * THERMISTOR_RESISTOR_VALUE / tempVoltage;
     uint16_t intResistance = (uint16_t) resistance;
     return intResistance;
    // resistances stored as 1 ohm units.
}

uint16_t thermistorResistanceGPIO3(double tempVoltage) {
    /* voltage measured across thermistor is dependent on the resistor in the voltage divider
     * v = 5 (Rt / (Rt + R1));
     * Rt*v + R1*v = 5*Rt;
     * Rt*v - 5*Rt = -R1*v;
     * Rt = (R1*v) / (5 - v);
     * all voltage measurements stored in arrays are in 0.1 mV, or 1/10,000 of a volt
     */
     /* NEW EQUATION
      */
    double resistance = (5e3 * THERMISTOR_RESISTOR_VALUE * tempVoltage) / (1e4 * (50000.0 - tempVoltage) - 5e3 * tempVoltage);
    return printResistance(resistance);
    // resistances stored as 1 ohm units.
}

uint16_t printResistance(double resistance) {
    Serial.print("RESISTANCE: "); Serial.println(resistance);
    uint16_t intResistance = (uint16_t) resistance;
    Serial.print("INT RESISTANCE: "); Serial.println(intResistance);
    return intResistance;
}

uint16_t calculateDegreesCelsius(double thermistorResistance) {
    // temperature equation based on resistance is the following
    // R_inf = R0 * e^(-B / T0);
    // T = B / ln((R/R0) * e^(B / T0))
    // T = B / (ln(R/R0) + ln(e^(B / T0)))
    // T = B / (ln(R/R0) + (B / T0))
    // B = 3984
    // R0 = 10000
    double temp = 3984 / (log(thermistorResistance / 1e4) + (3984.0 / 298.15));
    temp = temp - 273.15;
    Serial.println(temp);
    return (int) (temp * 100);
    // temps stored in 0.1 C units
}

float process_current() {
    // max positive current at 90% of 5V = 4.5V
    // max negative current in opposite direction at 10% of 5V = 0.5V
    // 0 current at 50% of 5V = 2.5V
    // max current sensor reading +/- 300A
    // current = 300 * (V - 2.5v) / 2v
    double senseVoltage = analogRead(CURRENT_SENSE) * 5.0 / 1024;
    float current = (float) MAX_VAL_CURRENT_SENSE * (senseVoltage - 2.5) / 2;
    Serial.print("Current: "); Serial.println(current);
    bmsCurrentMessage.setCurrent(current);
    if (current < 0) {
        bmsCurrentMessage.setChargingState(CHARGING);
    } else if (current > 0) {
        bmsCurrentMessage.setChargingState(DISCHARGING);
    }
    if (bmsCurrentMessage.getChargingState() == 0) { // discharging
        if (bmsCurrentMessage.getCurrent() > DISCHARGE_CURRENT_CONSTANT_HIGH) {
            bmsStatusMessage.setDischargeOvercurrent(true);
            Serial.println("DISCHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
        }
    } else if (bmsCurrentMessage.getChargingState() == 1) { // charging
        if (bmsCurrentMessage.getCurrent() < CHARGE_CURRENT_CONSTANT_HIGH) {
            bmsStatusMessage.setChargeOvercurrent(true);
            Serial.println("CHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
        }
    }
    return current;
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

void process_voltages() {
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
    bmsVoltageMessage.setAverage(static_cast<uint16_t>(avgVolt * 1000 + 0.5)); // stored in millivolts
    bmsVoltageMessage.setTotal(static_cast<uint16_t>(totalVolts + 0.5)); // number is in units volts
    minVolt = (minVolt + 5) / 10;
    maxVolt = (maxVolt + 5) / 10;
    bmsVoltageMessage.setLow(minVolt);
    bmsVoltageMessage.setHigh(maxVolt);

    // TODO: Low and High voltage error checking.
    if (bmsVoltageMessage.getHigh() > VOLTAGE_HIGH_CUTOFF) {
        bmsStatusMessage.setOvervoltage(true);
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
    }

    if (bmsVoltageMessage.getLow() < VOLTAGE_LOW_CUTOFF) {
        bmsStatusMessage.setUndervoltage(true);
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
    }
    if (bmsVoltageMessage.getTotal() > TOTAL_VOLTAGE_CUTOFF) {
        bmsStatusMessage.setTotalVoltageHigh(true);
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
    }

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

void writeToCAN() {
    Serial.println("WRITING TO CAN!");
    digitalWrite(10, HIGH);
    unsigned char msg[8] = {0,0,0,0,0,0,0,0};
    bmsVoltageMessage.write(msg);
    byte CANsendMsgResult = CAN.sendMsgBuf(ID_BMS_VOLTAGE, 0, 8, msg);

    bmsCurrentMessage.setChargingState(0b1);
    bmsCurrentMessage.setCurrent(77.777);
    bmsCurrentMessage.write(msg);
    CANsendMsgResult = CAN.sendMsgBuf(ID_BMS_CURRENT, 0, 8, msg);

    bmsTempMessage.write(msg);
    CANsendMsgResult = CAN.sendMsgBuf(ID_BMS_TEMPERATURE, 0, 8, msg);

    bmsStatusMessage.write(msg);
    CANsendMsgResult = CAN.sendMsgBuf(ID_BMS_STATUS, 0, 8, msg);
    digitalWrite(10, HIGH);
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
