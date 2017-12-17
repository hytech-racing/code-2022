/*
 * HyTech 2018 BMS Control Unit
 * Init 2017-04-11
 * Configured for HV Board Rev 2
 * Monitors cell voltages and temperatures, sends BMS_OK signal to close Shutdown Circuit
 */

#include <Arduino.h>
#include <FlexCAN.h>
#include "HyTech17.h"
#include "LTC68041.h"
#include <Metro.h>

/*
 * Pin definitions
 */
#define BMS_OK A8
#define CURRENT_SENSE A3
#define LED_STATUS 7
#define TEMP_SENSE_1 A9
#define TEMP_SENSE_2 A2
#define WATCHDOG A0

/*
 * Timers
 */
Metro timer_can_update = Metro(100);
Metro timer_debug = Metro(500);
Metro timer_watchdog_timer = Metro(250);

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
short voltage_cutoff_low = 2980;
short voltage_cutoff_high = 4210;
short total_voltage_cutoff = 150;
short discharge_current_constant_high = 220;
short charge_current_constant_high = -400;
short max_val_current_sense = 300;
short charge_temp_critical_high = 4400;// 44.00
short discharge_temp_critical_high = 6000; // 60.00
short voltage_difference_threshold = 500; //100 mV, 0.1V

#define ENABLE_CAN false // use this definition to enable or disable CAN
/********GLOBAL ARRAYS/VARIABLES CONTAINING DATA FROM CHIP**********/
#define TOTAL_IC 2 // DEBUG: We have temporarily overwritten this value
#define TOTAL_CELLS 9
#define TOTAL_THERMISTORS 3 // TODO: Double check how many thermistors are being used.
#define TOTAL_SEGMENTS 4
#define THERMISTOR_RESISTOR_VALUE 6700 // TODO: Double check what resistor is used on the resistor divider.
uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages. Numbers are stored in 0.1 mV units.
uint16_t aux_voltages[TOTAL_IC][6]; // contains auxiliary pin voltages.
     /* Data contained in this array is in this format:
      * Thermistor 1
      * Thermistor 2
      * Thermistor 3
      */
int16_t cell_delta_voltage[TOTAL_IC][TOTAL_CELLS]; // keep track of which cells are being discharged
int16_t ignore_cell[TOTAL_IC][TOTAL_CELLS]; //cells to be ignored for Balance testing

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
FlexCAN CAN(500000);
static CAN_message_t msg;

/**
 * BMS State Variables
 */
BMS_segment_voltages bms_segment_voltages[TOTAL_SEGMENTS]; // TODO write individual temperatures here for CAN bus
BMS_status bms_status;
BMS_temperatures bms_temperatures;
BMS_voltages bms_voltages;
bool watchdog_high = true;

int minVoltageICIndex;
int minVoltageCellIndex;
int voltage_difference;

bool cell_discharging[TOTAL_IC][12];

void setup() {
    pinMode(BMS_OK, OUTPUT);
    pinMode(WATCHDOG, OUTPUT);
    pinMode(10,OUTPUT); // Chip select pin
    digitalWrite(BMS_OK, HIGH);

    Serial.begin(115200); // Init serial for PC communication
    if (ENABLE_CAN) {
        CAN.begin();
    }
    delay(100);
    Serial.println("CAN system and serial communication initialized");

    digitalWrite(BMS_OK, HIGH);
    digitalWrite(WATCHDOG, watchdog_high);

    LTC6804_initialize();
    init_cfg();
    poll_cell_voltage();                                                                                                                                  
    memcpy(cell_delta_voltage, cell_voltages, 2 * TOTAL_IC * TOTAL_CELLS);
    bms_status.set_state(BMS_STATE_CHARGING);
    Serial.println("Setup Complete!");
    
    // DEBUG Code for testing cell packs
    /*ignore_cell[0][3] = true;
    ignore_cell[0][4] = true;
    ignore_cell[0][5] = true;
    ignore_cell[0][6] = true;*/
}

// TODO Implement Coulomb counting to track state of charge of battery.

/*
 * Main BMS Control Loop
 */
void loop() {
    /*if (ENABLE_CAN) {
        while (CAN.read(msg)) {
            if (msg.id == NULL) { // TODO replace with approate definition
                // lines set out for changing BMS variables TODO
                Serial.println("Reading BMS");
            }
        }    
    }*/
    // poll_cell_voltage(); No need to print this twice
    process_voltages(); // polls controller, and sto data in bms_voltages object.
    //bms_voltages.set_low(37408); // DEBUG Remove before final code
    balance_cells();
    process_temps(); // store data in bms_temperatures object.
    //process_current(); // sto data in bms_status object.
    
    if (!bms_status.get_error_flags()) { // set BMS_OK signal
        Serial.println("STATUS NOT GOOD!!!!!!!!!!!!!!!");
        digitalWrite(BMS_OK, LOW);
    }

    /*
    wakeFromSleepAllChips();
    tx_cfg[0][1] = 0b01010010;
    tx_cfg[0][2] = 0b10000111;
    tx_cfg[0][3] = 0b10100010;
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);*/
    wakeFromIdleAllChips();
    delay(10);
    wakeup_idle();

    /*
    uint8_t data[1][8];
    uint8_t cmd[2] = {0x00,0x12};
    int res = LTC6804_rdcfg(0,data);
    for (int i = 0;i<8;i++){
      Serial.print("register: ");
      Serial.print(i);
      Serial.print(" value: ");
      Serial.println(data[0][i]);

       Serial.print("PEC:");
       Serial.println(res);
    }
    delay(2000);
    */

    if (ENABLE_CAN && timer_can_update.check()) {
        bms_status.write(msg.buf);
        msg.id = ID_BMS_STATUS;
        msg.len = sizeof(CAN_message_bms_status_t);
        CAN.write(msg);

        bms_voltages.write(msg.buf);
        msg.id = ID_BMS_VOLTAGES;
        msg.len = sizeof(CAN_message_bms_voltages_t);
        CAN.write(msg);

        msg.id = ID_BMS_SEGMENT_TEMPERATURES;
        msg.len = sizeof(CAN_message_bms_segment_voltages_t);
        for (int i = 0; i < TOTAL_SEGMENTS; i++) {
            bms_segment_voltages[i].write(msg.buf);
            CAN.write(msg);
        }

        bms_temperatures.write(msg.buf);
        msg.id = ID_BMS_TEMPERATURES;
        msg.len = sizeof(CAN_message_bms_temperatures_t);
        CAN.write(msg);
    }

    if (timer_watchdog_timer.check()) { // Send alternating keepalive signal to watchdog timer
        watchdog_high = !watchdog_high;
        digitalWrite(WATCHDOG, watchdog_high);
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
        tx_cfg[i][1] = 0x52 ;
        tx_cfg[i][2] = 0x87 ;
        tx_cfg[i][3] = 0xA2 ;
        tx_cfg[i][4] = 0x00 ;
        tx_cfg[i][5] = 0x00 ;
    }
    wakeFromSleepAllChips();
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
    // dischargeAll();
}

void discharge_cell(int ic, int cell) {
    cell_discharging[ic][cell] = true;
    discharge_cell(ic, cell, true);
}

void discharge_cell(int ic, int cell, bool setDischarge) {
    if (ic < TOTAL_IC && cell < TOTAL_CELLS) {
        if (cell < 8) {
            if(setDischarge){
                tx_cfg[ic][4] = tx_cfg[ic][4] | (0b1 << cell); 
            }else{
                tx_cfg[ic][4] = tx_cfg[ic][4] & ~(0b1 << cell ); 
            }
        } else {
            if(setDischarge){
                tx_cfg[ic][5] = tx_cfg[ic][5] | (0b1 << (cell - 8)); 
            }else{
                tx_cfg[ic][5] = tx_cfg[ic][5] & ~(0b1 << (cell - 8)); 
            }
        }
    }
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
    wakeFromSleepAllChips();
}

void dischargeAll() {
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][4] = 0b11111111;
        tx_cfg[i][5] = tx_cfg[i][5] | 0b00001111;
    }
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
    wakeFromSleepAllChips();
}

void stop_discharge_cell(int ic, int cell)
{
    cell_discharging[ic][cell] = false;
    discharge_cell(ic, cell, false);
}

void stop_dischargeAll() {
    for (int i = 0; i < TOTAL_IC; i++) {
        tx_cfg[i][4] = 0b0;
        tx_cfg[i][5] = 0b0;
    }
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
    wakeFromSleepAllChips();
}


void balance_cells () {
  if (bms_voltages.get_low() > voltage_cutoff_low)
  { 
      for (int ic = 0; ic < TOTAL_IC; ic++)
      { // for IC
          for (int cell = 0; cell < TOTAL_CELLS; cell++)
          { // for Cell
              if (!ignore_cell[ic][cell])
              {
                  uint16_t cell_voltage = cell_voltages[ic][cell]; // current cell voltage in mV
                  if (cell_discharging[ic][cell])
                  {
                      if (cell_voltage < bms_voltages.get_low() + voltage_difference_threshold - 6)
                      {
                          stop_discharge_cell(ic, cell);
                      }
                  }
                  else if (cell_voltage > bms_voltages.get_low() + voltage_difference_threshold)
                      {
                          discharge_cell(ic, cell);
                  }
              }
          }
      }
  }
  else
  {
      Serial.println("Not Balancing!");
      stop_dischargeAll();
      //make sure none of the cells are discharging
  }
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
    printCells(); // prints the cell voltages to Serial.
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
            if ((ic != 0 || cell != 4) && (ic != 1 || cell != 7)&&!ignore_cell[ic][cell]) {
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
    bms_voltages.set_average(static_cast<uint16_t>(avgVolt * 1000 + 0.5)); // stored in millivolts
    bms_voltages.set_total(static_cast<uint16_t>(totalVolts + 0.5)); // number is in units volts
    bms_voltages.set_low(minVolt);
    bms_voltages.set_high(maxVolt);

    // TODO: Low and High voltage error checking.

    bms_status.set_overvoltage(false); // RESET these values, then check below if they should be set again
    bms_status.set_undervoltage(false);
    bms_status.set_total_voltage_high(false);
    
    if (bms_voltages.get_high() > voltage_cutoff_high*10) {
        bms_status.set_overvoltage(true);
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
        Serial.print("max IC: "); Serial.println(maxIC);
        Serial.print("max Cell: "); Serial.println(maxCell); Serial.println();
    }

    if (bms_voltages.get_low() < voltage_cutoff_low) {
        bms_status.set_undervoltage(true);
        Serial.println("VOLTAGE FAULT!!!!!!!!!!!!!!!!!!!");
        Serial.print("min IC: "); Serial.println(minIC);
        Serial.print("min Cell: "); Serial.println(minCell); Serial.println();
    }
    if (bms_voltages.get_total() > total_voltage_cutoff) {
        bms_status.set_total_voltage_high(true);
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
    bms_temperatures.set_low_temperature((uint16_t) lowTemp);
    bms_temperatures.set_high_temperature((uint16_t) highTemp);
    bms_temperatures.set_average_temperature((uint16_t) avgTemp);

    bms_status.set_discharge_overtemp(false); // RESET these values, then check below if they should be set again
    bms_status.set_charge_overtemp(false);

    if (bms_status.get_state() == BMS_STATE_DISCHARGING) { // Discharging
        if (bms_temperatures.get_high_temperature() > discharge_temp_critical_high) {
            bms_status.set_discharge_overtemp(true);
            Serial.println("TEMPERATURE FAULT!!!!!!!!!!!!!!!!!!!");
        }
    } else if (bms_status.get_state() == BMS_STATE_CHARGING) { // Charging
        if (bms_temperatures.get_high_temperature() > charge_temp_critical_high) {
            bms_status.set_charge_overtemp(true);
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
    float current = (float) max_val_current_sense * (senseVoltage - 2.5) / 2;
    Serial.print("Current: "); Serial.println(current);
    bms_status.set_current(current); // TODO convert number to uint16_t correctly
    bms_status.set_charge_overcurrent(false); // RESET these values, then check below if they should be set again
    bms_status.set_discharge_overcurrent(true);
    if (current < 0) {
        bms_status.set_state(BMS_STATE_CHARGING);
        if (bms_status.get_current() < charge_current_constant_high) { // TODO make sure number gets converted properly
            bms_status.set_charge_overcurrent(true);
            Serial.println("CHARGE CURRENT HIGH FAULT!!!!!!!!!!!!!!!!!!!");
        }
    } else if (current > 0) {
        bms_status.set_state(BMS_STATE_DISCHARGING);
        if (bms_status.get_current() > discharge_current_constant_high) { // TODO make sure number gets converted properly
            bms_status.set_discharge_overcurrent(true);
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

int updateConstraints(uint8_t address, short value) {
    switch(address) {
        case 0: // voltage_cutoff_low
            voltage_cutoff_low = value;
            break;
        case 1: // voltage_cutoff_high
            voltage_cutoff_high = value;
            break;
        case 2: // total_voltage_cutoff
            total_voltage_cutoff = value;
            break;
        case 3: // discharge_current_constant_high
            discharge_current_constant_high = value;
            break;
        case 4: // charge_current_constant_high
            charge_current_constant_high = value;
            break;
        case 5: // max_val_current_sense
            max_val_current_sense = value;
            break;
        case 6: // charge_temp_critical_high
            charge_temp_critical_high = value;
            break;
        case 7: // discharge_temp_critical_high
            discharge_temp_critical_high = value;
            break;
        case 8: // voltage_difference_threshold
            voltage_difference_threshold = value;
            break;
        default:
            return -1;
    }
    return 0;
}

void printCells() {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
        Serial.print("IC: ");
        Serial.println(current_ic+1);
        for (int i = 0; i < TOTAL_CELLS; i++) {
            Serial.print("C"); Serial.print(i);
            if (ignore_cell[current_ic][i]) {
                Serial.print(" IGNORED CELL ");
            }
            Serial.print(": ");
            float voltage = cell_voltages[current_ic][i] * 0.0001;
            Serial.print(voltage, 4);
            Serial.print(" Discharging: ");
            Serial.print(cell_discharging[current_ic][i]); 
            Serial.print(" Voltage difference: ");
            Serial.print(cell_voltages[current_ic][i]-bms_voltages.get_low());
            Serial.print(" Delta To Threshold: ");
            Serial.println((cell_voltages[current_ic][i]-bms_voltages.get_low())-voltage_difference_threshold);
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
