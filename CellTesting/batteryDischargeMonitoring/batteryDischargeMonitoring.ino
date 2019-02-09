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
#include <ADC_SPI.h>
#include <Metro.h>

int timestep = 20; //datalog timestep (milliseconds)
bool pulsing = false; // if pulsing should be used in middle of cycle
int pulses = 4; // if pulsing is set to TRUE, then number of pulses to be included in cycle

// arrays for storing information about cells
double cell_voltage[4];    // cells' voltage reading
double cell_current[4];    // cells' current reading
double cell_resistance[4]; // cells' resistance
double cell_OCV[4];        // Calculated cell OCV
double cell_dvdt[4];       // Numerical derivative of voltage change
double cell_Ah_inst[4];    // One-timestep coulomb counting
double cell_Ah[4];         // Running total of coulomb count for total amp-hours
double cell_Wh_inst[4];    // One-timestep watt-hours
double cell_Wh[4];         // Running total of watt-hours

double test_resistance[4]; // Calculated resistance of the discharge power resistor

const double R = 1; // the value of the resistor hooked up to cells


// the process of testing can be in 3 states: WAIT, DISCHARGE, and DONE. The following vatriables are used to keep track of the state of each cell.
int state[4]; // stores the state of each cell
const int WAIT = 0; //
const int DISCHARGE = 1;
const int DONE = 2;
int contactor_command[4];

int start_delay = 5000; // milliseconds
int end_delay = 10000; // milliseconds


// assign teensy pins
const int CONTACTOR_PWR_SENSE = 20;
const int SWITCH[4] = {15, 16, 17, 18};
const int CONTACTOR_VLT_THRESHOLD = 474; // 7 V
const double END_VOLTAGE = 3;


// conversion factors for calculating voltage and current
int i = 0;
double voltage_conversion_factor = 5.0 / 4095; // determined by testing
double current_conversion_factor = 150 / 1.5;  // L01Z150S05 current sensor outputs 4 V at 150 A, and 2.5 V at 0 A


bool contactor_voltage_high = false; // indicates whether the contactor is powered or not (true = powered)
unsigned int contactor_voltage = 0; // reading on the CONTACTOR_PWR_SENSE (the value is between 0 and 1023, it is NOT in volts)
unsigned long start_millis = 0; // stores the value of millis() when the discharging started
unsigned long end_millis = 0; // stores the value of millis() when the discharging ended
bool millis_started = false;    // stores whether start_millis has been recorded or not


// create an adc object
ADC_SPI adc;

// create a metro timer object
Metro timer = Metro(timestep, 1); // return true 50 times per second; ignore missed calls;

void CellDataLog(int channel) {
  if (timer.check()) {
    // print data to Serial in the format "CH#,V,I,R,t"
    Serial.print(i + 1);                Serial.print(", ");
    Serial.print(cell_voltage[i]);      Serial.print(", ");
    Serial.print(cell_current[i]);      Serial.print(", ");
    Serial.print(test_resistance[i]);   Serial.print(", ");
    Serial.print(contactor_command[i]); Serial.print(", ");
    Serial.println(millis() - start_millis);
  }
}

// method to read the cell voltage in VOLTS
// Arguments: channel (Note: in the code, channels are numbered 0-3, when on the board they are 1-4)
double getBatteryVoltage(int channel) {
  double voltage_reading = ((double) adc.read_adc(channel)) * voltage_conversion_factor;
  return voltage_reading;
}

// method to read the cell current in Amps
// Arguments: channel (Note: in the code, channels are numbered 0-3, when on the board they are 1-4)
double getBatteryCurrent(int channel) {
  double voltage_reading = ((double) adc.read_adc(channel + 4)) * voltage_conversion_factor;
  double current_reading = (voltage_reading - 2.5) * current_conversion_factor;
  return current_reading;
}


void setup() {
  adc = ADC_SPI();
  Serial.begin(115200);
  delay(1000); // need a delay after Serial.begin()

  // set the mode for SWITCH pins and set them to LOW. When SWITCH pins are low, the relays are open, and cells are not discharging
  for (int i = 0; i < 4; i++) {
    pinMode(SWITCH[i], OUTPUT);
    contactor_command[i] = 0;
    digitalWrite(SWITCH[i], contactor_command[i]);
  }

  pinMode(CONTACTOR_PWR_SENSE, INPUT); // set the contactor pin as INPUT

  // set the default state for all cells as WAIT
  for (int i = 0; i < 4; i++) {
    state[i] = WAIT;
  }
}

void loop() {

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Read Inputs
  contactor_voltage = analogRead(CONTACTOR_PWR_SENSE); // read contactor voltage

  for (int i = 0; i < 4; i++) {

    cell_voltage[i] = getBatteryVoltage(i); // read cell's voltage
    cell_current[i] = getBatteryCurrent(i);
    test_resistance[i] = cell_voltage[i] / cell_current[i];

    if (cell_voltage[i] <= END_VOLTAGE) { // END CONDITION: set to end if read voltage low
      state[i] = DONE;
      end_millis = millis();
    }
    ///////////////////////////////////////////////////////////////////////////////////////////

    // Decide action based on state cell is in
    if (state[i] == DONE) {
      contactor_command[i] = 0;
      digitalWrite(SWITCH[i], contactor_command[i]);
      //record data for 10 seconds after finished
      if (millis() - end_millis <= end_delay) {
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
        start_millis = millis();
      }
    }

    else if (state[i] == DISCHARGE) {

      if (millis() - start_millis <= start_delay) { // wait for start delay before closing relay
        contactor_command[i] = 0;
        digitalWrite(SWITCH[i], contactor_command[i]);
      }

      else {
        contactor_command[i] = 1;
        digitalWrite(SWITCH[i], contactor_command[i]);
      }

      CellDataLog(i);

    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
