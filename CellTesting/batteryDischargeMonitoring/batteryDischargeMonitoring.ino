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

double END_VOLTAGE  = 3.000;      // voltage threshold for end of test

const int    timestep      = 20;         //datalog timestep (milliseconds)
bool         pulsing_on    = true;       // if pulsing should be used in middle of cycle
int          V_end         = 2;          // 0 = instantaneous voltage ends cycle, 1 = rolling average window ends cycle, 2 = predicted OCV ends cycle
bool         manual_offset = false;     // use manual offset for current sensor voltage offset

int    start_delay  = 5 * 1000;   // time to log data before start milliseconds
int    end_delay    = 45 * 1000;  // time to log data after end milliseconds

const int    channels   = 4;
const int    rollingwin = 100;     // rolling average window needs to be small compared to pulse time & battery time constant for IR calculations

const int    delimiter = 1;       // 0 for comma (CSV), 1 for tab

//*****************************************************************************************
//********CONFIGURATION********************************************************************
//*****************************************************************************************


//////////Pulsing setup////////////////////////////////////////////////////////////////////
const int     num_pulses        = 5;         // if pulsing is set to TRUE, then number of pulses to be included in cycle
int     pulse_int         = 30 * 1000; // milliseconds to wait for pulse off cycle // pulse interval will likely need to be greater than rolling average window
double  pulsing_threshold = 0.020;     // Volts/second threshold used to determine linear region; steep region is approx 0.1 V/s
int     pulses_completed[channels]  = {0,0,0,0};         // keeping track of pulses
bool    pulsing[channels]           = {false,false,false,false};      // pulsing flag
unsigned long pulse_time[channels]  = {0,0,0,0};         // for timing purposes
//////////Pulsing setup////////////////////////////////////////////////////////////////////


//////////Data Storage/////////////////////////////////////////////////////////////////////
//////////Arrays for storing information about cells

//////////Real time Parameters
double v_read[channels];                         // cells' voltage reading overwritten per iteration
double i_read[channels];                         // cells' current reading overwritten per iteration

double cell_voltage[channels][rollingwin];      // cells' voltage reading w/ history per timestep
double cell_current[channels][rollingwin];      // cells' current reading w/ history per timestep

double cell_voltage2[channels][rollingwin];     // cells' voltage reading (temp array for shifting)
double cell_current2[channels][rollingwin];     // cells' current reading (temp array for shifting)
double v_avg[channels];                               // used for calculating rolling avg
double i_avg[channels];                               // used for calculating rolling avg
double v_last[channels];                               // used for calculating cell IR
double i_last[channels];                               // used for calculating cell IR

double test_resistance[channels];      // Calculated resistance of the discharge power resistor
int    contactor_command[channels];     // Digital high/low used for contactor (used for post-processing)

//////////Calculated Parameters
double cell_dvdt[channels];            // Numerical derivative of voltage change
double cell_didt[channels];            // Numerical derivative of current change
double cell_IR[channels][num_pulses];  // Calculated cell internal resistance for each pulse
double cell_IR_avg[channels];          // Calculated cell internal resistance (averaged)
double cell_OCV[channels];             // Calculated cell open circuit voltage

double cell_Ah_inst[channels];         // One-timestep coulomb counting
double cell_Ah[channels];              // Running total of coulomb count for total amp-hours
double cell_Wh_inst[channels];         // One-timestep watt-hours
double cell_Wh[channels];              // Running total of watt-hours
//////////Data Storage/////////////////////////////////////////////////////////////////////


//////////State Machine/////////////////////////////////////////////////////////////////////
//////////Each channel can be in 4 states: WAIT -> DISCHARGE -> DONE | NOCELL
//////////The following vatriables are used to keep track of the state of each cell.
int state[channels]; // stores the state of each cell
int low_v[channels];

const int WAIT        = 0;
const int DISCHARGE   = 1;
const int DONE        = 2;
const int NOCELL      = 3;

bool            contactor_voltage_high  = false;   // indicates whether the contactor is powered or not (true = powered)
unsigned int    contactor_voltage       = 0;       // reading on the CONTACTOR_PWR_SENSE (the value is between 0 and 1023, it is NOT in volts)

bool            started[channels]              = {false};  // stores if channel has started testing
unsigned long   test_time[channels]            = {0};     // stores whether start_millis has been recorded or not
unsigned long   start_millis[channels]         = {1306,1306,1306,1306};     // stores the value of millis() when the discharging started
signed long   end_millis[channels]           = {0};     // stores the value of millis() when the discharging ended
//////////State Machine/////////////////////////////////////////////////////////////////////


//////////Hardware Config/////////////////////////////////////////////////////////////////////
#include <ADC_SPI.h>
#include <Metro.h>
//////////Create an adc object
ADC_SPI adc;
//////////Create a metro timer object
Metro timer[channels]  = Metro(timestep, 1); // return true based on timestep period; ignore missed calls;

//////////Assign teensy pins
const int CONTACTOR_PWR_SENSE = 20;
const int SWITCH[channels] = {A1, A2, A3, A4}; //{15, 16, 17, 18};
const int CONTACTOR_VLT_THRESHOLD = 474; // 7 V

//////////Conversion factors for calculating voltage and current
double voltage_conversion_factor      = 5.033333333333333333 / 4095;   // determined by testing
double current_conversion_factor      = 150   / 1.500;  // L01Z150S05 current sensor outputs 4 V at 150 A, and 2.5 V at 0 A
long int    current_offset[4]         = {0,0,0,0};      // Used to determine the zero current offset of the current senesor, by default it is 2.500V
int    calibration_reads              = 500;            // number of readings for 0 offset calibration

//////////Hardware Config/////////////////////////////////////////////////////////////////////


//////////Functions///////////////////////////////////////////////////////////////////////////
void CellDataLog(int i) {

  if (timer[i].check()) {

    // Make a copy of the array to prepare for shifting
    for (int j = 0; j < rollingwin; j++) {
      cell_voltage2[i][j] = cell_voltage[i][j];
      cell_current2[i][j] = cell_current[i][j];
    }
    // Shift array by one index at every timestep
    for (int j = 0; j < rollingwin - 1; j++) {
      cell_voltage[i][j + 1] = cell_voltage2[i][j];
      cell_current[i][j + 1] = cell_current2[i][j];
    }
    cell_voltage[i][0] = v_read[i]; // update first value
    cell_current[i][0] = i_read[i];

    // Calculating rolling average
    v_avg[i] = 0;
    i_avg[i] = 0;
    for (int j = 0; j < rollingwin; j++) {
      v_avg[i] = v_avg[i] + cell_voltage[i][j];
      i_avg[i] = i_avg[i] + cell_current[i][j];
    }
    v_avg[i] = v_avg[i] / rollingwin;
    i_avg[i] = i_avg[i] / rollingwin;

    cell_dvdt[i] = (cell_voltage[i][0] - cell_voltage[i][rollingwin - 1]) / (timestep * rollingwin / (double)1000);
    cell_didt[i] = (cell_current[i][0] - cell_current[i][rollingwin - 1]) / (timestep * rollingwin / (double)1000);

    cell_OCV[i] = v_avg[i] + (i_avg[i] * cell_IR_avg[i]); // print calculated OCV

    //Print data to Serial in the format "CH#,t,V,I,R,Com,State

    //Comma Delimited:
    if      (delimiter == 0){
      Serial.print(i+1); Serial.print(",");Serial.print(test_time[i]); Serial.print(",");   Serial.print(cell_voltage[i][0],4); Serial.print(","); Serial.print(cell_current[i][0]); Serial.print(","); Serial.print(cell_OCV[i],4); Serial.print(",");    Serial.print(cell_IR_avg[i]*1000,4); Serial.print(",");Serial.print(contactor_command[i]); Serial.print(","); Serial.println(state[i]); // Serial.print(",");
    }
    //Tab Delimited:
    else if (delimiter == 1){
      Serial.print(i+1); Serial.print("\t");Serial.print(test_time[i]); Serial.print("\t");   Serial.print(cell_voltage[i][0],4); Serial.print("\t"); Serial.print(cell_current[i][0]); Serial.print("\t"); Serial.print(cell_OCV[i],4); Serial.print("\t");   Serial.print(cell_IR_avg[i]*1000,4); Serial.print("\t"); Serial.print(contactor_command[i]); Serial.print("\t"); Serial.println(state[i]); // Serial.print(",");
    }
  }
}

double getBatteryVoltage(int channel) {
  // Method to read the cell voltage in VOLTS
  // Arguments: channel (Note: in the code, channels are numbered 0-3, when on the board they are 1-4, used for voltage sense)
  double voltage_reading = ((double) adc.read_adc(channel)) * voltage_conversion_factor;
  return voltage_reading;
}

double getBatteryCurrent(int channel) {
  // Method to read the cell current in Amps
  // Arguments: channel (Note: in the code, channels are numbered 0-3, when on the board they are 4-7 which are designated as current sense)
  double voltage_reading = ((double) adc.read_adc(channel + 4)) * voltage_conversion_factor;
  double current_reading = (voltage_reading - (current_offset[channel]*voltage_conversion_factor)) * current_conversion_factor;
  return current_reading;
}
//////////Functions///////////////////////////////////////////////////////////////////////////

void setup() {
  adc = ADC_SPI();
  Serial.begin(115200);
  delay(1000); // need a delay after Serial.begin()

  // Set the contactor pin as INPUT
  pinMode(CONTACTOR_PWR_SENSE, INPUT);

  for (int i = 0; i < channels; i++) {

    // Set the mode for SWITCH pins and set them to LOW. When SWITCH pins are low, the relays are open, and cells are not discharging
    contactor_command[i] = 0;
    pinMode(SWITCH[i], OUTPUT);
    digitalWrite(SWITCH[i], contactor_command[i]);

    // Set the default state for all cells as WAIT
    state[i] = WAIT;

    // Used to find the 0 A offset for the current sensors
    for (int j = 0; j < calibration_reads; j++){
     current_offset[i] = current_offset[i] + adc.read_adc(i + 4);
    }
    current_offset[i] = round(current_offset[i] / (double)calibration_reads);

    if (manual_offset){
    current_offset[i] = 2048;
    }

    
    for (int j = 0; j < rollingwin; j++) { // fill entire array with same value
      cell_voltage[i][j]         = getBatteryVoltage(i); // read cell's voltage
      cell_current[i][j]         = getBatteryCurrent(i);
      cell_voltage2[i][j]        = cell_voltage[i][j]; // save in temp array
      cell_current2[i][j]        = cell_current[i][j];
    }

    v_avg[i] = cell_voltage[i][0];
    i_avg[i] = cell_current[i][0];

    cell_OCV[i] = v_avg[i]; // initialize OCV estimate
    
  }

  // Print Column Headers
  if      (delimiter == 0){
    Serial.print("Channel"); Serial.print(",");    Serial.print("Time"); Serial.print(",");    Serial.print("Voltage"); Serial.print(",");   Serial.print("Current"); Serial.print(",");   Serial.print("OCV Predict"); Serial.print(",");    Serial.print("Calculated IR mOhm"); Serial.print(",");Serial.print("Contactor"); Serial.print(","); Serial.println("State"); // Serial.print(", ");
  }
  else if (delimiter == 1){
    Serial.print("Channel"); Serial.print("\t");    Serial.print("Time"); Serial.print("\t");    Serial.print("Voltage"); Serial.print("\t");   Serial.print("Current"); Serial.print("\t");   Serial.print("OCV Predict"); Serial.print("\t");   Serial.print("Calculated IR mOhm"); Serial.print("\t"); Serial.print("Contactor"); Serial.print("\t"); Serial.println("State"); // Serial.print(", ");
  }
}

void loop() {

  contactor_voltage = analogRead(CONTACTOR_PWR_SENSE); // read contactor voltage

  for (int i = 0; i < channels; i++) {
    //////////Get Data///////////////////////////////////////////////////////////////////////////
    test_time[i]        = millis() - start_millis[i];
    //Serial.println(start_millis[i]);

    v_read[i]     = getBatteryVoltage(i); // read cell's voltage
    i_read[i]     = getBatteryCurrent(i); // read cell's current
    test_resistance[i]  = v_read[i] / i_read[i];

    if (V_end == 0 && v_read[i] <= END_VOLTAGE && state[i] != DONE) { // END CONDITION: set to end if read voltage low (instantaneous reading)
      end_millis[i] = millis();
      if (state[i] == WAIT){
        end_millis[i] = -1*end_delay;
      }
      state[i] = DONE; 
    }
    if (V_end == 1 && v_avg[i] <= END_VOLTAGE && state[i] != DONE) { // END CONDITION: set to end if read voltage low (rolling average reading)
      end_millis[i] = millis();
      if (state[i] == WAIT){
        end_millis[i] = -1*end_delay;
      }
      state[i] = DONE; 
    }
    if (V_end == 2 && cell_OCV[i] <= END_VOLTAGE && state[i] != DONE) { // END CONDITION: set to end if read voltage low (rolling average reading)
      end_millis[i] = millis();
      if (state[i] == WAIT){
       end_millis[i] = -1*end_delay;
      }
      state[i] = DONE; 
    }


    ///////////////////////////////////////////////////////////////////////////////////////////
    //////////Decide action based on state cell is in
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

      if (contactor_voltage < CONTACTOR_VLT_THRESHOLD) {
        state[i] = WAIT;
      }

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
          if      (pulses_completed[i] == 0 && millis() - start_delay - start_millis[i] <= rollingwin * timestep) { // allow enough time for one full rolling average window to populate
            contactor_command[i] = 1;
            digitalWrite(SWITCH[i], contactor_command[i]);
          }
          // pulse interval will likely need to be greater than rolling average window
          else if (!pulsing[i] && pulses_completed[i] < num_pulses && abs(cell_dvdt[i]) <= pulsing_threshold && millis() - pulse_time[i] > pulse_int) { // if not in a pulse, and have pulses left, and in linear cell discharge region, and enough time for rolling average window to populate
            v_last[i] = v_avg[i]; // rolling average window needs to be small compared to pulse time & battery time constant
            i_last[i] = i_avg[i];


            pulsing[i] = true;
            contactor_command[i] = 0;
            digitalWrite(SWITCH[i], contactor_command[i]);
            pulse_time[i] = millis();
            pulses_completed[i]++;;
          }
          else if (pulsing[i] && millis()-pulse_time[i] >= pulse_int ) { // if in the middle of a pulse and exceeded the time
            v_last[i] = v_avg[i] - v_last[i]; //difference from before, assume 0 current when contactor is open
            cell_IR[i][pulses_completed[i]-1] = v_last[i] / i_last[i];

            cell_IR_avg[i] = 0;
            for (int k = 0; k < pulses_completed[i]; k++){
              cell_IR_avg[i] = cell_IR_avg[i] + cell_IR[i][k];
            }
            cell_IR_avg[i] = (cell_IR_avg[i] / pulses_completed[i]); // resistance must be positive
            
            pulsing[i] = false;
            contactor_command[i] = 1;
            digitalWrite(SWITCH[i], contactor_command[i]);
            pulse_time[i] = millis();        
          }
        }
      }
      CellDataLog(i);
    }
  }
}
