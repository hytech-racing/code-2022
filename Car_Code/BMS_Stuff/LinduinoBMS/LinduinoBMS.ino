/**
 * LinduinoBMS.ino - Code that runs on the BMS Linduino. This system only has one Linduino, which interfaces directly with the Car's CAN Bus
 * Created by Shrivathsav Seshan & Charith "Karvin" Dassanayake, April 11, 2017
 */

#include <Arduino.h>
#include "mcp_can.h"
#include "LTC68041.h"
#include "HyTech17.h"

/*
 * 
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
#define CHARGE_CURRENT_CONSTANT_HIGH -400
#define MAX_VAL_CURRENT_SENSE 300
#define CHARGE_TEMP_CRITICAL_HIGH 4400 // 44.00
#define DISCHARGE_TEMP_CRITICAL_HIGH 6000 // 60.00

/********GLOBAL ARRAYS/VARIABLES CONTAINING DATA FROM CHIP**********/
#define TOTAL_IC 4
#define TOTAL_CELLS 9
#define TOTAL_THERMISTORS 3 // TODO: Double check how many thermistors are being used.
#define THERMISTOR_ISTOR_VALUE 6700 // TODO: Double check what istor is used on the istor divider.
uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages. Sto numbers in 0.1 mV units.
uint16_t aux_voltages[TOTAL_IC][6]; // contains auxiliary pin voltages.
     /* Data contained in this array is in this format:
      * Thermistor 1
      * Thermistor 2
      * Thermistor 3
      */
int16_t cell_delta_voltage[TOTAL_IC][12]; // contains 12 signed dV values in 0.1 mV units

/*!<
  The tx_cfg[][6] sto the LTC6804 configuration data that is going to be written
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
#define CURRENT_SENSE 2
bool watchDogFlag = true;
BMS_voltages bmsVoltageMessage;
BMS_currents bmsCurrentMessage;
BMS_temperatures bmsTempMessage;
BMS_status bmsStatusMessage;

int minVoltageICIndex;
int minVoltageCellIndex;

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
    poll_cell_voltage();
    memcpy(cell_delta_voltage, cell_voltages, 2 * TOTAL_IC * TOTAL_CELLS);
    bmsCurrentMessage.setChargingState(CHARGING);
    Serial.println("Setup Complete!");
}

// NOTE: Implement Coulomb counting to track state of charge of battery.
/*
 * Main BMS Control Loop
 */

void loop() {
    process_voltages(); // polls controller, and sto data in bmsVoltageMessage object.

    process_temps(); // sto datap in bmsTempMessage object.
    process_current(); // sto data in bmsCurrentMessage object.

    // write to CAN!
    writeToCAN();

    // set BMS_OK signal
    if (!bmsStatusMessage.getBMSStatusOK()) {
        Serial.println("STATUS NOT GOOD!!!!!!!!!!!!!!!");
        digitalWrite(BMS_OK_PIN, LOW);
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

void poll_cell_voltage() {
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
    uint8_t error = LTC6804_rdcv(0, TOTAL_IC, cell_voltages); // asks chip to read voltages and sto in given array.
    if (error == -1) {
        Serial.println("A PEC error was detected in cell voltage data");
    }
//    printCells(); // prints the cell voltages to Serial.
    delay(200); // TODO: Why 200 milliseconds?
}

void process_voltages() {
    poll_cell_voltage(); // cell_voltages[] array populated with cell voltages now.
    double totalVolts = 0; // stored as double volts
    uint16_t maxVolt = cell_voltages[0][0]; // stored in 0.1 mV units
    uint16_t minVolt = cell_voltages[0][0]; // stored in 0.1 mV units
    double avgVolt = 0; // stored as double volts
    int maxIC = 0;
    int maxCell = 0;
    int minIC = 0;
    int minCell = 0;
    for (int ic = 0; ic < TOTAL_IC; ic++) {
        for (int cell = 0; cell < TOTAL_CELLS; cell++) {
            if ((ic != 0 || cell != 4) && (ic != 1 || cell != 7)) {
                uint16_t currentCell = cell_voltages[ic][cell];
                cell_delta_voltage[ic][cell] = currentCell - cell_delta_voltage[ic][cell];
                if (currentCell > maxVolt) {
                    maxVolt = currentCell;
                    maxIC = ic;
                    maxCell = cell;
                }
                if (currentCell < minVolt) {
                    minVolt = currentCell;
                    minIC = ic;
                    minCell = cell;
                }
                totalVolts += currentCell * 0.0001;
            }
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
        Serial.print("max IC: "); Serial.println(maxIC);
        Serial.print("max Cell: "); Serial.println(maxCell); Serial.println();
    }

    if (bmsVoltageMessage.getLow() < VOLTAGE_LOW_CUTOFF) {
        bmsStatusMessage.setUndervoltage(true);
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
        Serial.print("min IC: "); Serial.println(minIC);
        Serial.print("min Cell: "); Serial.println(minCell); Serial.println();
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

void poll_aux_voltage() {
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
    poll_aux_voltage();
    totalTemp = 0;
    lowTemp = calculateDegreesCelsius(thermistorResistanceGPIO12(aux_voltages[0][0]));
    highTemp = lowTemp;
    for (int ic = 0; ic < TOTAL_IC; ic++) {
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
        resistance = thermistorResistanceGPIO3(aux_voltages[ic][2]);
        thermTemp = calculateDegreesCelsius(resistance);
        if (thermTemp < lowTemp) {
            lowTemp = thermTemp;
        }
        if (thermTemp > highTemp) {
            highTemp = thermTemp;
        }
        totalTemp += thermTemp;
        Serial.println("----------------------\n");
    }
    avgTemp = (uint16_t) (totalTemp / ((3) * TOTAL_THERMISTORS));
    bmsTempMessage.setLowTemp((uint16_t) lowTemp);
    bmsTempMessage.setHighTemp((uint16_t) highTemp);
    bmsTempMessage.setAvgTemp((uint16_t) avgTemp);

    if (bmsCurrentMessage.getChargingState() == 0) { // discharging
        if (bmsTempMessage.getHighTemp() > DISCHARGE_TEMP_CRITICAL_HIGH) {
            bmsStatusMessage.setDischargeOvertemp(true);
            Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
        }
    } else if (bmsCurrentMessage.getChargingState() == 1) { // charging
        if (bmsTempMessage.getHighTemp() > CHARGE_TEMP_CRITICAL_HIGH) {
            bmsStatusMessage.setChargeOvertemp(true);
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

/*
 * tempVoltage is a double in unit 0.1mV
 */
uint16_t thermistorResistanceGPIO12(double tempVoltage) {
    /* voltage measured across thermistor is dependent on the istor in the voltage divider
     * all voltage measurements stored in arrays are in 0.1 mV, or 1/10,000 of a volt
     */
     tempVoltage = tempVoltage / 1e4;
     double resistance = 1e6 * (5 - tempVoltage) / (tempVoltage + 100 * tempVoltage - 5);
     Serial.println(resistance, 2);
     return (uint16_t) resistance;
    // resistances stored as 1 ohm units.
}

/*
 * tempVoltage is a double in units volts
 */
static inline uint16_t thermistorResistanceGPIO3(double tempVoltage) {
    /* voltage measured across thermistor is dependent on the istor in the voltage divider
     * all voltage measurements stored in arrays are in 0.1 mV, or 1/10,000 of a volt
     */
    tempVoltage = tempVoltage / 1e4;
    Serial.println(tempVoltage);
    double res = 5000.0 * tempVoltage;
    Serial.print("Step 1: "); Serial.println(res, 2);
    res = 25000.0 - res;
    Serial.print("Step 2: "); Serial.println(res, 2);
    res = res + 5000.0;
    Serial.print("Step 3: "); Serial.println(res, 2);
    res = res / (tempVoltage - 1.0);
    Serial.print("Final Step 4: "); Serial.println(res, 2);
//    double res = (25000.0 - 5000.0 * tempVoltage + 5000.0) / (tempVoltage - 1.0);
    Serial.print("resistance 3: "); Serial.println(res, 2);
    uint16_t small_res = (uint16_t) res;
    Serial.print("integer resistance 3: "); Serial.println(small_res);
    return small_res;
    // resistances stored as 1 ohm units.
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
        if (bmsCurrentMessage.getCurrent() < CHARGE_CURRENT_CONSTANT_HIGH) {
            bmsStatusMessage.setChargeOvercurrent(true);
            Serial.println("CHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
        }
    } else if (current > 0) {
        bmsCurrentMessage.setChargingState(DISCHARGING);
        if (bmsCurrentMessage.getCurrent() > DISCHARGE_CURRENT_CONSTANT_HIGH) {
            bmsStatusMessage.setDischargeOvercurrent(true);
            Serial.println("DISCHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
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

void writeToCAN() {
    Serial.println("WRITING TO CAN!");
    digitalWrite(10, HIGH);
    unsigned char msg[8] = {0,0,0,0,0,0,0,0};
    bmsVoltageMessage.write(msg);
    byte CANsendMsgresult = CAN.sendMsgBuf(ID_BMS_VOLTAGE, 0, 8, msg);

    bmsCurrentMessage.setChargingState(0b1);
    bmsCurrentMessage.setCurrent(77.777);
    bmsCurrentMessage.write(msg);
    CANsendMsgresult = CAN.sendMsgBuf(ID_BMS_CURRENT, 0, 8, msg);

    bmsTempMessage.write(msg);
    CANsendMsgresult = CAN.sendMsgBuf(ID_BMS_TEMPERATURE, 0, 8, msg);

    bmsStatusMessage.write(msg);
    CANsendMsgresult = CAN.sendMsgBuf(ID_BMS_STATUS, 0, 8, msg);
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
