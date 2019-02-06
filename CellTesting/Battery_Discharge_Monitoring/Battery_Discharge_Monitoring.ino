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

// assign teensy pins
const int CONTACTOR_PWR_SENSE = 20;

const int SWITCH[4] = {15, 16, 17, 18};
const int CONTACTOR_VLT_THRESHOLD = 474; // 7 V
const double END_VOLTAGE = 3;

const double R = 1; // the value of the resistor hooked up to cells

// conversion factors for calculating voltage and current
double voltage_conversion_factor = 5.0 / 4095; // determined by testing
double current_conversion_factor = 150 / 1.5;  // L01Z150S05 current sensor outputs 4 V at 150 A, and 2.5 V at 0 A

// arrays for storing information about cells
double cell_voltage[4];    // cells' voltage reading
double cell_current[4];    // cells' current reading
double cell_resistance[4]; // cells' resistance

bool contactor_voltage_high = false; // indicates whether the contactor is powered or not (true = powered)

unsigned int contactor_voltage = 0; // reading on the CONTACTOR_PWR_SENCE (the value is between 0 and 1023, it is NOT in volts)

unsigned long start_millis = 0; // stores the value of millis() when the discharging started
bool millis_started = false;    // stores whether start_millis has been recorded or not

// the process of testing can be in 3 states: WAIT, DISCHARGE, and DONE. The following vatriables are used to keep track of the state of each cell.
int state[3]; // stores the state of each cell
const int WAIT = 0; //
const int DISCHARGE = 1;
const int DONE = 2;

// create an adc object
ADC_SPI adc;

// create a metro timer objects
Metro read_timer = Metro(20, 1);           // return true 50 times per second; ignore missed calls;
Metro pulse_start_timer = Metro(30000, 1); // return true evey 5 mins; ignore missed calls;
Metro pulsing_timer = Metro(3000, 1);      // return true every 30 seconds; ignore missed calls;

void setup() {
  adc = ADC_SPI();
  Serial.begin(115200);
  delay(1000); // need a delay after Serial.begin()

  // set the mode for SWITCH pins and set them to LOW. When SWITCH pins are low, the relays are open, and cells are not discharging
  for (int i = 0; i < 4; i++) {
    pinMode(SWITCH[i], OUTPUT);
    digitalWrite(SWITCH[i], LOW);
  }

  pinMode(CONTACTOR_PWR_SENSE, INPUT); // set the contactor pin as INPUT

  // set the default state for all cells as WAIT
  for (int i = 0; i < 4; i++) {
    state[i] = WAIT;
  }
}

void loop() {

  contactor_voltage = analogRead(CONTACTOR_PWR_SENSE); // read contactor voltage

  // if the contactor voltage below threshold, set states of all cells to WAIT
  if (contactor_voltage < CONTACTOR_VLT_THRESHOLD) {
    for (int i = 0; i < 4; i++) {
      state[i] = WAIT;
    }
  // else, record data
  } else {

    for (int i = 0; i < 4; i++) {

      cell_voltage[i] = getBatteryVoltage(i); // read cell's voltage

      if (read_timer.check()) {
        if (cell_voltage[i] > END_VOLTAGE && state[i] != DONE) { // check if the cell is above end voltage and it is not already done discharging
          // if the current state is WAIT, change the state to DISCHARGE
          if (state[i] == WAIT) {
            state[i] = DISCHARGE;

            // if start_millis has not yet been recorded, record it, and set millis_started to true
            if (!millis_started) {
              start_millis = millis();
              millis_started = true;
            }
          }

          digitalWrite(SWITCH[i], HIGH); // close the relay to start discharging

          // record current and internal resistance
          cell_current[i] = getBatteryCurrent(i);
          // cell_resistance[i] = cell_voltage[i] / cell_current[i] - R;

          // print data to Serial in the format "CH#,V,I,t"
          Serial.print(i + 1); Serial.print(", "); Serial.print(cell_voltage[i]); Serial.print(", "); Serial.print(cell_current[i]);
          Serial.print(", "); /*Serial.print(cell_resistance[i]); Serial.print(", ");*/ Serial.println(millis() - start_millis);

        // check if the cell voltage is below the end voltage
        } else if (cell_voltage[i] < END_VOLTAGE) {

          // if the current state is DISCHARGE, change the state to DONE
          if (state[i] == DISCHARGE) {
            state[i] = DONE;
          }

          digitalWrite(SWITCH[i], LOW); // open the relay to stop discharging
        }
      }
    }
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
