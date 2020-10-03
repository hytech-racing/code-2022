// #define OUTPUT_MODE std::ios::app // append data
#define OUTPUT_MODE std::ios::trunc // overwrite data

#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <time.h>
#include "can_msg_def.h"

#include <vector>

using namespace std;

void analyze(time_t _ts, int srcId, int messageLen, unsigned long long _message, fstream& output) {
  unsigned long long message = 0;
  for (int i = 0; i < messageLen; i++) {
    message = (message << 8) + (_message & 0xFF);
    _message >>= 8;
  }
  
  vector<definition> msg_definition = CAN_MSG_DEFINITION[srcId].second;

  struct tm* ptm = gmtime(&_ts);
  char ts [20];
  sprintf(&ts[0], "%d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

  for(definition& d : msg_definition) {
    vector<bool> map;
    long long parsedData = d.parse(message, messageLen, map);
    if(map.size()) {
      if (srcId ==  0xAB) {
        for(int i = 0; i < d.booleanMappings.size(); i++) {
          if (map[i])
            output << ts << ",MC FAULT: " << d.booleanMappings[i] << "," << map[i] << endl;
        }
      }
      else {
        for (int i = 0; i < d.booleanMappings.size(); i++) {
          output << ts << "," << d.booleanMappings[i] << "," << map[i] << endl;
        }
      }
    }
    else {
      output << ts << "," << d.field << "," << (d.multiplier ? d.multiplier * parsedData : parsedData);
      if(d.units != "") output << "," << d.units;
      output << endl;
    }
  }
}

fstream getFile(string function, ios_base::openmode mode, bool secondTry = false) {
  fstream file;

  if(secondTry) cout << "Error opening " << function << " file. ";
  while(true) {
    cout << "Enter path to " << function << " file: ";
    string filepath; getline(cin, filepath);
    if (filepath.length() < 4 || (filepath.compare(filepath.length() - 4, 4, ".csv") && filepath.compare(filepath.length() - 4, 4, ".CSV")))
      filepath += ".csv";
    file.open(filepath, mode);
    if (file.is_open()) return file;
    cout << "Error opening " << function << " file. ";
  }
}

int main(int argc, char* argv[]){
  loadLookupTable();
  fstream in, out;

  if(argc > 1) in.open(argv[1], ios::in);
  if(argc > 2) out.open(argv[2], ios::out | OUTPUT_MODE);

  if (!in.is_open()) in = getFile("input", ios::in, argc > 1);
  if (!out.is_open()) out = getFile("output", ios::out | OUTPUT_MODE, argc > 2);

  in.ignore(256, '\n');
  out << "Timestamp, Field, Value, Units\n";

  string inputLine;
  while(in.good()) {
    getline(in, inputLine);
    stringstream lineStream(inputLine);
    time_t timestamp;
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

  return 0;
}
