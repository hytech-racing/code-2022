
#include <string.h>
#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "can_msg_def.h"

using namespace std;

//Information logged by 1 data point in the CSV file
struct dataPoint{
    double time;
    unsigned int ID;
    int length;
    unsigned long long response;
};

//Helper function to convert string in CSV file to hex code stored as an int
unsigned long long parseHex(string &s) {
  stringstream ss;
  ss << hex << s;
  unsigned long long x; ss >> x;
  return x;
}

int readAsHex(istream &in) {
    cout<<"im here"<<endl;
    int x;
    in >> hex >> x;
    return x;
}

void readCSV(string filename, vector<vector<dataPoint>> &byteArray){
    ifstream  data(filename); //creates a filestream to read data from CSV
    string line;
    vector<dataPoint> tempCSV; //Unprocessed data, converted straight from the CSV file to this vector

    getline(data, line); //Skips first line of the CSV file (heading info)

    int o = 0;
    while(getline(data,line)){ //obtains every line in CSV except for 1st line
        stringstream lineStream(line);  //Create a stringstream and pushes the line from CSV to it
        string cell;
        vector<string> parsedRow;
        while(getline(lineStream,cell,',')){ //seperates the line of CSV into individual string elements seperated by commas
            parsedRow.push_back(cell); //adds elements to string vector
        }
        tempCSV.push_back(dataPoint()); //creates space in memory to add a dataPoint
        //Conversion and assignment of unformatted string to tempCSV with proper formating
        tempCSV[o].time     = stod(parsedRow[0]);
        tempCSV[o].ID       = parseHex(parsedRow[1]);
        tempCSV[o].length   = stoi(parsedRow[2]);
        tempCSV[o].response = parseHex(parsedRow[3]);
        o++;
    }

    //all CAN IDs being logged on SD Card, stored as strings, add to this in the same order as shown if more CAN ID values exist
    vector<string> string_hex = {"A0","A1","A2","A3","A4","A5","A6","A7","A8","A9","AA","AB","AC","AD","AE","C0","C3","C4","CC","D7","D8","D9","DA","DB","DE","DF","E2","E7","E8"};
    vector<int> int_hex;
    int i = 0;
    while(i < string_hex.size()){ //converts string_hex to their respective interger hex values for comparison and sorting later
        int_hex.push_back(parseHex(string_hex[i]));
        i++;
    }

    int size_tempCSV = static_cast<int>(tempCSV.size()); //when obtaining size of a 2D vector of struct, static_cast is required
    int y = 0;
    int z = 0;

    while(y<size_tempCSV){ //iterates through every element in tempCSV
       while(z<int_hex.size()){
           if(tempCSV[y].ID == int_hex[z]){//finds the mating ID value between int_hex and tempCSV, the value of this determines where the dataPoint will be stored in the 2D Vector
                byteArray[z].push_back(tempCSV[y]); //push back the z-th row with the dataPoint stored at tempCSV[y]
                break; //exit remaining iterations to save time
           }
           z++;
       }
       z=0;
       y++;
    }
}

/*
0     "A0", ID_MC_TEMPERATURES_1
1     "A1", ID_MC_TEMPERATURES_2
2     "A2", ID_MC_TEMPERATURES_3
3     "A3", ID_MC_ANALOG_INPUTS_VOLTAGES
4     "A4", ID_MC_DIGITAL_INPUT_STATUS
5     "A5", ID_MC_MOTOR_POSITION_INFORMATION
6     "A6", ID_MC_CURRENT_INFORMATION
7     "A7", ID_MC_VOLTAGE_INFORMATION
8     "A8", ID_MC_FLUX_INFORMATION
9     "A9", ID_MC_INTERNAL_VOLTAGES
10    "AA", ID_MC_INTERNAL_STATES
11    "AB", ID_MC_FAULT_CODES
12    "AC", ID_MC_TORQUE_TIMER_INFORMATION
13    "AD", ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION
14    "AE", ID_MC_FIRMWARE_INFORMATION
15    "C0", ID_MC_COMMAND_MESSAGE
16    "C3", ID_MCU_STATUS
17    "C4", ID_MCU_PEDAL_READINGS
18    "CC", ID_GLV_CURRENT_READINGS
19    "D7", ID_BMS_VOLTAGE
20    "D8", ID_BMS_DETAILED_VOLTAGES
21    "D9", ID_BMS_TEMPERATURES
22    "DA", ID_DETAILED_TEMPERATURES
23    "DB", ID_BMS_STATUS
24    "DE", ID_BMS_BALANCING_STATUS
25    "DF", ID_FCU_ACCELEROMETER
26    "E2", ID_BMS_COULOMB_COUNTS
27    "E7", ID_MCU_GPS_READINGS_ALPHA
28    "E8"  ID_MCU_GPS_READINGS_GAMMA
*/

/* CAN IDs with known fields
0  "A0"
1  "A1"
2  "A2"
3  "A3"
4  "A4"
5  "A5"
6  "A6"
7  "A7"
10 "AA"
11 "AB"
12 "AC"
13 "AD"
14 "AE"
15 "C0"
16 "C3"
17 "C4"
19 "D7"
20 "D8"
21 "D9"
22 "DA"
23 "DB"
24 "DE"
25 "E2"
*/
void dataAnalyze(vector<vector<dataPoint>> &csv){
    //definition(int o, int l, TYPE_CAST t, string f, string d, vector<string> m = {})

    vector<definition> MCU_PEDAL_READINGS = CAN_MSG_DEFINITION[0xC4].second;

    unsigned long long message = 0xB5074408A0010000;
    for(definition d : MCU_PEDAL_READINGS) {
        vector<bool> map;
        int parsedData = d.parse(message, 8, map);
        if(map.size()) {
            for (int i = 0; i < d.booleanMappings.size(); i++) {
                cout << d.booleanMappings[i] << ": " << map[i] << endl;
            }
        } else {
            cout << d.field << ": " << parsedData << endl;
        }
    }
}

void analyze(double ts, int srcId, int messageLen, unsigned long long message, ofstream& output) {
  vector<definition> msg_definition = CAN_MSG_DEFINITION[srcId].second;
  ts *= 1000;

  for(definition d : msg_definition) {
    vector<bool> map;
    int parsedData = d.parse(message, messageLen, map);
    if(map.size()) {
      for(int i = 0; i < d.booleanMappings.size(); i++) {
        output << setprecision (numeric_limits<double>::digits10 + 1) << ts
          << "," << d.booleanMappings[i] << "," << map[i] << endl;
      }
    }
    else {
      output << setprecision (numeric_limits<double>::digits10 + 1) << ts
          << "," << d.field << "," << parsedData << endl;
    }
  }
}

int main(){
    loadLookupTable();
    ifstream in; ofstream out;

    // ofstream tout ("./test");
    // analyze(0, 0xAA, 8, 0x400090000000001ULL, tout);
    // tout.close();

    do {
      cout << "Enter path to input file: ";
      string filepath; getline(cin, filepath);
      if(filepath.length() < 4 || filepath.compare(filepath.length() - 4, 4, ".csv"))
        filepath += ".csv";
      filepath = "./DATA0000.CSV";
      in = ifstream(filepath);
      if(!in.is_open()) cout << "Could not open file to read.";
    } while(!in.is_open());
    in.ignore(256, '\n');

    do {
      cout << "Enter path to output file: ";
      string filepath; getline(cin, filepath);
      if(filepath.length() < 4 || filepath.compare(filepath.length() - 4, 4, ".csv"))
        filepath += ".csv";
      filepath = "./DATA0000_PARSED.csv";
      out = ofstream (filepath);
      if(!out.is_open()) cout << "Could not create file to write.";
    } while(!out.is_open());

    out << "Timestamp, Field, Value, Units\n";

    string inputLine;
    while(in.good()) {
      getline(in, inputLine);
      stringstream lineStream(inputLine);
      double timestamp;
      int srcId, length;
      unsigned long long message;
      lineStream >> timestamp;
      lineStream.ignore(256, ',');
      lineStream >> hex >> srcId;
      lineStream.ignore(256, ',');
      lineStream >> length;
      lineStream.ignore(256, ',');
      lineStream >> message;

      analyze(timestamp, srcId, length, message, out);
    }
    in.close();
    out.close();
}

/* example definition - MCU pedal readings = 0xC4
* bytelen: 2, signed: false, field: "accelerator_pedal_raw_1"
* bytelen: 2, signed: false, field: "accelerator_pedal_raw_2"
* bytelen: 2, signed: false, field: "brake_pedal_raw",
* bytelen: 1, signed: false, field: "pedal_flags", booleanMappings: ["accelrator_implausibility", "brake_implausibility", "brake_pedal_active"]
* bytelen: 1, signed: false, field: "torque_map_mode"
*/
