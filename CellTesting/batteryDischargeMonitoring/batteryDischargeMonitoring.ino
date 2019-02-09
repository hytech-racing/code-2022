/*
  Leonid Pozdneev
  Created January 2019

  This code controls Battery Discharge Monitoring Board. It collects voltage, current and internal resistance
  data from 4 channels on the board and then prints it to Arduino Serial in the format "CH#, Voltage, Current, Resistance, time(milliseconds)"
  (.csv file format). In order to process the data, the data needs to be copied from the Serial window and saved in a .csv file.

  In this code, each of 4 channels of the board can be in 3 states: WAIT, DISCHARGE, DONE.
  All channels are in WAIT state if the contactor is not powered. When the contactor is powered,
  if the voltage on a channel is above 3 Volts, the channel enters DISCHARGE state. In DISCHARGE state,
  the contactor relay is closed which allowes cell to runn current and discharge (this is when data is
  printed to the Serial port). Once the voltage on channel goes below the set threshold, it is put into
  DONE state, the relay is opened, and the printing of data is stopped.
  data r
*/

//*****************************************************************************************
//********CONFIGURATION********************************************************************
//*****************************************************************************************

double END_VOLTAGE = 3.000;      // voltage threshold for end of test

int    timestep   = 20;          //datalog timestep (milliseconds)
bool   pulsing_on = false;       // if pulsing should be used in middle of cycle

int    start_delay = 5 * 1000;   // time to log data before start milliseconds
int    end_delay   = 10 * 1000;  // time to log data after end milliseconds

//*****************************************************************************************
//********CONFIGURATION********************************************************************
//*****************************************************************************************


//////////Pulsing setup////////////////////////////////////////////////////////////////////
int     pulses_completed  = 0;        //keeping track of pulses
double  pulsing_threshold = 0.050;    // Volts/second threshold used to determine linear rgion
int     pulse_int         = 30 * 1000; // milliseconds to wait for pulse off cycle
int     num_pulses        = 4;        // if pulsing is set to TRUE, then number of pulses to be included in cycle
//////////Pulsing setup////////////////////////////////////////////////////////////////////


//////////Data Storage/////////////////////////////////////////////////////////////////////
//////////Arrays for storing information about cells

//////////Realatime Parameters
double cell_voltage[4];         // cells' voltage reading
double cell_current[4];         // cells' current reading
double test_resistance[4];      // Calculated resistance of the discharge power resistor
int    contactor_command[4]     // Digital high/low used for contactor (used for post-processing)

//////////Calculated Parameters
double cell_prev_voltage[4];    // cells' voltage reading at last timestep
double cell_prev_current[4];    // cells' current reading at last timestep
double cell_now_voltage[4];     // cells' voltage reading at current timestep
double cell_now_current[4];     // cells' current reading at current timestep

double cell_dvdt[4];            // Numerical derivative of voltage change
double cell_didt[4];            // Numerical derivative of current change
double cell_resistance[4];      // cells' resistance
double cell_OCV[4];             // Calculated cell OCV

double cell_Ah_inst[4];         // One-timestep coulomb counting
double cell_Ah[4];              // Running total of coulomb count for total amp-hours
double cell_Wh_inst[4];         // One-timestep watt-hours
double cell_Wh[4];              // Running total of watt-hours
//////////Data Storage/////////////////////////////////////////////////////////////////////


//////////State Machine/////////////////////////////////////////////////////////////////////
//////////Each channel can be in 4 states: WAIT -> DISCHARGE -> DONE | NOCELL
//////////The following vatriables are used to keep track of the state of each cell.
int i = 0;
int state[4]; // stores the state of each cell

const int WAIT        = 0;
const int DISCHARGE   = 1;
const int DONE        = 2;
const int NOCELL      = 3;

bool            contactor_voltage_high  = false;   // indicates whether the contactor is powered or not (true = powered)
unsigned int    contactor_voltage       = 0;       // reading on the CONTACTOR_PWR_SENSE (the value is between 0 and 1023, it is NOT in volts)

bool            started[4]              = {false}  // stores if channel has started testing
unsigned long   test_time[4]            = {0};     // stores whether start_millis has been recorded or not
unsigned long   start_millis[4]         = {0};     // stores the value of millis() when the discharging started
unsigned long   end_millis[4]           = {0};     // stores the value of millis() when the discharging ended
//////////State Machine/////////////////////////////////////////////////////////////////////


//////////Hardware Config/////////////////////////////////////////////////////////////////////
#include <ADC_SPI.h>
#include <Metro.h>
//////////Create an adc object
ADC_SPI adc;
//////////Create a metro timer object
Metro timer  = Metro(timestep, 1); // return true based on timestep period; ignore missed calls;
Metro timer2 = Metro(timestep, 1); // return true based on timestep period; ignore missed calls;

//////////Assign teensy pins
const int CONTACTOR_PWR_SENSE = 20;
const int SWITCH[4] = {A1, A2, A3, A4}; //{15, 16, 17, 18};
const int CONTACTOR_VLT_THRESHOLD = 474; // 7 V

//////////Conversion factors for calculating voltage and current
double voltage_conversion_factor = 5.000 / 4095;   // determined by testing
double current_conversion_factor = 150   / 1.500;  // L01Z150S05 current sensor outputs 4 V at 150 A, and 2.5 V at 0 A
int    current_vref[10]          = {2048};            // Used to determine the zero current offset of the current senesor, by default it is 2.500V
//////////Hardware Config/////////////////////////////////////////////////////////////////////


//////////Functions///////////////////////////////////////////////////////////////////////////
void CellDataCalc(int channel) {
  if (timer2.check()) {
    cell_prev_voltage[i] = cell_now_voltage[i];
    cell_prev_current[i] = cell_now_current[i];

    cell_now_voltage[i] = cell_voltage[i];
    cell_now_current[i] = cell_current[i];

    cell_dvdt[i] = (cell_now_voltage - cell_prev_voltage) / (timestep / 1000);
    cell_didt[i] = (cell_now_current - cell_prev_current) / (timestep / 1000);

  }
}

void CellDataLog(int channel) {
  if (timer.check()) {  // print data to Serial in the format "CH#,t,V,I,R,Com,State"
//    Serial.print("Channel");Serial.print(", ");    Serial.print("C1 Time");Serial.print(", ");     Serial.print("C1 Voltage");Serial.print(", ");    Serial.print("C1 Current"); Serial.print(", ");   Serial.print("C1 Test Resistance");Serial.print(", ");  Serial.print("C1 Contactor"); Serial.print(", ");      Serial.print("C1 State"); Serial.print(", ");
      Serial.print(channel]);Serial.print(",");      Serial.print(test_time[i]);Serial.print(",");   Serial.print(cell_voltage[i]);Serial.print(", "); Serial.print(cell_current[i]);Serial.print(", "); Serial.print(test_resistance[i]);Serial.print(", ");    Serial.print(contactor_command[i]);Serial.print(", "); Serial.println("C1 State");// Serial.print(", ");
  }
}

double getBatteryVoltage(int channel) {
  // Method to read the cell voltage in VOLTS
  // Arguments: channel (Note: in the code, channels are numbered 0-3, when on the board they are 1-4)
  double voltage_reading = ((double) adc.read_adc(channel)) * voltage_conversion_factor;
  return voltage_reading;
}

double getBatteryCurrent(int channel) {
  // Method to read the cell current in Amps
  // Arguments: channel (Note: in the code, channels are numbered 0-3, when on the board they are 1-4)
  double voltage_reading = ((double) adc.read_adc(channel + 4)) * voltage_conversion_factor;
  double current_reading = (voltage_reading - 2.5) * current_conversion_factor;
  return current_reading;
}
//////////Functions///////////////////////////////////////////////////////////////////////////


void setup() {
  adc = ADC_SPI();
  Serial.begin(115200);
  delay(1000); // need a delay after Serial.begin()

  // Set the contactor pin as INPUT
  pinMode(CONTACTOR_PWR_SENSE, INPUT);

  for (int i = 0; i < 4; i++) {

    // Set the mode for SWITCH pins and set them to LOW. When SWITCH pins are low, the relays are open, and cells are not discharging
    contactor_command[i] = 0;
    pinMode(SWITCH[i], OUTPUT);
    digitalWrite(SWITCH[i], contactor_command[i]);

    // Set the default state for all cells as WAIT
    state[i] = WAIT;
    cell_voltage[i] = getBatteryVoltage(i); // read cell's voltage
    cell_current[i] = getBatteryCurrent(i);
    cell_prev_voltage[i] = cell_voltage[i]; // save cell's voltage (initialize)
    cell_prev_current[i] = cell_current[i];
    cell_now_voltage[i] = cell_voltage[i]; // save cell's voltage (initialize)
    cell_now_current[i] = cell_current[i];
  }

 // Print Column Headers
 Serial.print("Channel");Serial.print(", ");    Serial.print("Time");Serial.print(", ");    Serial.print("Voltage");Serial.print(", ");   Serial.print("Current"); Serial.print(", ");   Serial.print("Test Resistance");Serial.print(", ");    Serial.print("Contactor"); Serial.print(", "); Serial.println("State");// Serial.print(", ");
}

void loop() {

  //////////Get Data///////////////////////////////////////////////////////////////////////////
  contactor_voltage = analogRead(CONTACTOR_PWR_SENSE); // read contactor voltage

  for (int i = 0; i < 4; i++) {
  //////////Get Data///////////////////////////////////////////////////////////////////////////
    test_time[i]        = millis() - start_millis();
    cell_voltage[i]     = getBatteryVoltage(i); // read cell's voltage
    cell_current[i]     = getBatteryCurrent(i);
    test_resistance[i]  = cell_voltage[i] / cell_current[i];

    if (cell_voltage[i] <= END_VOLTAGE && state[i] != DONE) { // END CONDITION: set to end if read voltage low
      state[i] = DONE;
      end_millis[i] = millis();
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    CellDataCalc(i);
    ///////////////////////////////////////////////////////////////////////////////////////////

    // Decide action based on state cell is in
    if (state[i] == DONE) {
      contactor_command[i] = 0;
      digitalWrite(SWITCH[i], contactor_command[i]);
      //record data for 10 seconds after finished
      if (millis() - end_millis[i] <= end_delay) {
        CellDataLog(i);
      }
    }

    else if (state[i] == WAIT) {
      //if state is WAIT then change state based on contactor voltage threshold
      //Contactor voltage must be high the entire duration of the test
      if (contactor_voltage < CONTACTOR_VLT_THRESHOLD) {
        state[i] = WAIT;
      }
      else if (contactor_voltage >= CONTACTOR_VLT_THRESHOLD) {
        state[i] = DISCHARGE;
        start_millis[i] = millis();
      }
    }

    else if (state[i] == DISCHARGE) {

      if (millis() - start_millis[i] <= start_delay) { // wait for start delay before closing relay
        contactor_command[i] = 0;
        digitalWrite(SWITCH[i], contactor_command[i]);
      }

      else {
        if      (!pulsing_on) {
          contactor_command[i] = 1;
          digitalWrite(SWITCH[i], contactor_command[i]);
        }
        else if (pulsing_on)  {

        }
      }
      
      CellDataLog(i);

    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
