#ifndef CAN_MSG_DEF_H
#define CAN_MSG_DEF_H

#include <vector>

using namespace std;

struct definition {
    int offset;
    int len;
    bool isSigned = false;
    double multiplier = 0;
    string field = "";
    string units = "";
    vector<string> booleanMappings;

    definition(int o, int l, bool s, string f, string u = "", double m = 0) {
      offset = o;
      len = l;
      isSigned = s;
      field = f;
      units = u;
      multiplier = m;
    }

    definition(int o, int l, vector<string> m) {
      offset = o;
      len = l;
      booleanMappings = m;
    }

    long long parse(unsigned long long _u, int messageLen, vector<bool> &boolmap) {
        unsigned long long u = 0;
        for (int i = 0; i < messageLen; i++) {
          u = (u << 8) + (_u & 0xFF);
          _u >>= 8;
        }

        unsigned long long mask = (1ULL << (8 * len)) - 1;
        long long rawData = (u >> (8*(messageLen-offset-len))) & mask;

        if (!booleanMappings.empty()) {
            boolmap = vector<bool>(8*len);
            mask = 1ULL << (8*len - 1);
            for(int i = 0; i < booleanMappings.size(); i++) {
                boolmap[i] = (bool)(mask & rawData);
                mask >>= 1;
            }
        } else if(isSigned) {
            mask = 1ULL << (8*len - 1);
            if((rawData & mask) != 0) {
                mask = (mask << 1) - 1;
                return rawData | ~mask;
            }
        }
        return rawData;
    }
};

static vector<pair<string, vector<definition>>> CAN_MSG_DEFINITION (0x100, pair<string, vector<definition>>("ERROR: NO DEFINITION FOUND", {}));

static void loadLookupTable() {
  CAN_MSG_DEFINITION[0xA0] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_1", {
    definition(6, 2, true, "MODULE A TEMP", "C", 0.1),
    definition(4, 2, true, "MODULE B TEMP", "C", 0.1),
    definition(2, 2, true, "MODULE C TEMP", "C", 0.1),
    definition(0, 2, true, "GATE DRIVER BOARD TEMP", "C", 0.1)
  });
  CAN_MSG_DEFINITION[0xA1] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_2", {
    definition(6, 2, true, "CONTROL BOARD TEMPERATURE", "C", 0.1),
    definition(4, 2, true, "RTD 1 TEMPERATURE", "C", 0.1),
    definition(2, 2, true, "RTD 2 TEMPERATURE", "C", 0.1),
    definition(0, 2, true, "RTD 3 TEMPERATURE", "C", 0.1)
  });
  CAN_MSG_DEFINITION[0xA2] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_3", {
    definition(6, 2, true, "RTD 4 TEMP", "C", 0.1),
    definition(4, 2, true, "RTD 5 TEMP", "C", 0.1),
    definition(2, 2, true, "MOTOR TEMP", "C", 0.1),
    definition(0, 2, true, "TORQUE SHUDDER", "C", 0.1)
  });
  CAN_MSG_DEFINITION[0xA3] = pair<string, vector<definition>> ("ID_MC_ANALOG_INPUTS_VOLTAGES", {
    definition(6, 2, true, "ANALOG INPUT 1"),
    definition(4, 2, true, "ANALOG INPUT 2"),
    definition(2, 2, true, "ANALOG INPUT 3"),
    definition(0, 2, true, "ANALOG INPUT 4")
  });
  CAN_MSG_DEFINITION[0xA4] = pair<string, vector<definition>> ("ID_MC_DIGITAL_INPUT_STATUS", {
    definition(0, 1, {"DIGITAL INPUT 1", "DIGITAL INPUT 2", "DIGITAL INPUT 3", "DIGITAL INPUT 4", "DIGITAL INPUT 5", "DIGITAL INPUT 6", "DIGITAL INPUT 7", "DIGITAL INPUT 8"})
  });
  CAN_MSG_DEFINITION[0xA5] = pair<string, vector<definition>> ("ID_MC_MOTOR_POSITION_INFORMATION", {
    definition(6, 2, true, "MOTOR ANGLE", "", 0.1),
    definition(4, 2, true, "MOTOR SPEED", "RPM"),
    definition(2, 2, true, "ELEC OUTPUT FREQ", "", 0.1),
    definition(0, 2, true, "DELTA RESOLVER FILT")
  });
  CAN_MSG_DEFINITION[0xA6] = pair<string, vector<definition>> ("ID_MC_CURRENT_INFORMATION", {
    definition(6, 2, true, "PHASE A CURRENT", "A", 0.1),
    definition(4, 2, true, "PHASE B CURRENT", "A", 0.1),
    definition(2, 2, true, "PHASE C CURRENT", "A", 0.1),
    definition(0, 2, true, "DC BUS CURRENT", "A", 0.1)
  });
  CAN_MSG_DEFINITION[0xA7] = pair<string, vector<definition>> ("ID_MC_VOLTAGE_INFORMATION", {
    definition(6, 2, true, "DC BUS VOLTAGE", "V", 0.1),
    definition(4, 2, true, "OUTPUT VOLTAGE", "V", 0.1),
    definition(2, 2, true, "PHASE AB VOLTAGE", "V", 0.1),
    definition(0, 2, true, "PHASE BC VOLTAGE", "V", 0.1)
  });
  CAN_MSG_DEFINITION[0xA8] = pair<string, vector<definition>> ("ID_MC_FLUX_INFORMATION", {});
  CAN_MSG_DEFINITION[0xA9] = pair<string, vector<definition>> ("ID_MC_INTERNAL_VOLTAGES", {});
  CAN_MSG_DEFINITION[0xAA] = pair<string, vector<definition>> ("ID_MC_INTERNAL_STATES", {
    definition(6, 2, false, "VSM STATE"),
    definition(5, 1, false, "INVERTER STATE"),
    definition(4, 1, false, "INVERTER RUN MODE"),
    definition(3, 1, false, "INVERTER ACTIVE DISCHARGE STATE"),
    definition(2, 1, false, "INVERTER COMMAND MODE"),
    definition(1, 1, false, "INVERTER ENABLE"),
    definition(0, 1, false, "DIRECTION COMMAND")
  });
  CAN_MSG_DEFINITION[0xAB] = pair<string, vector<definition>> ("ID_MC_FAULT_CODES", {
    definition(6, 2, false, "POST FAULT LO"),
    definition(4, 2, false, "POST FAULT HI"),
    definition(2, 2, false, "RUN FAULT LO"),
    definition(0, 2, false, "RUN FAULT HI")
  });
  CAN_MSG_DEFINITION[0xAC] = pair<string, vector<definition>> ("ID_MC_TORQUE_TIMER_INFORMATION", {
    definition(4, 2, true, "COMMANDED TORQUE", "Nm", 0.1),
    definition(2, 2, true, "TORQUE FEEDBACK", "Nm", 0.1),
    definition(0, 4, false, "RMS UPTIME", "s", 0.003)
  });
  CAN_MSG_DEFINITION[0xAD] = pair<string, vector<definition>> ("ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION", {
    definition(6, 2, false, "MODULATION INDEX"),
    definition(4, 2, true, "FLUX WEAKENING OUTPUT"),
    definition(2, 2, true, "ID COMMAND"),
    definition(0, 2, true, "IQ COMMAND")
  });
  CAN_MSG_DEFINITION[0xAE] = pair<string, vector<definition>> ("ID_MC_FIRMWARE_INFORMATION", {
    definition(6, 2, false, "eeprom_version_project_code"),
    definition(4, 2, false, "SOFTWARE VERSION"),
    definition(2, 2, false, "DATE CODE MMDD"),
    definition(0, 2, false, "DATE CODE YYYY")
  });
  CAN_MSG_DEFINITION[0xC0] = pair<string, vector<definition>> ("ID_MC_COMMAND_MESSAGE", {
    definition(6, 2, true, "TORQUE COMMAND", "", 0.1),
    definition(4, 2, true, "ANGULAR VELOCITY"),
    definition(3, 1, {"DIRECTION"}),
    definition(2, 1, false, "INVERTER ENABLE DISCHARGE ENABLE"),
    definition(0, 2, true, "COMMANDED TORQUE LIST")
  });
  CAN_MSG_DEFINITION[0xC1] = pair<string, vector<definition>> ("ID_MC_READ_WRITE_PARAMETER_COMMAND", {
    definition(6, 2, false, "PARAMETER ADDRESS"),
    definition(5, 1, {"RW COMMAND"}),
    definition(4, 1, false, "RESERVED"),
    definition(0, 4, false, "DATA")
  });
  CAN_MSG_DEFINITION[0xC2] = pair<string, vector<definition>> ("ID_MC_READ_WRITE_PARAMETER_RESPONSE", {
    definition(6, 2, false, "PARAMETER ADDRESS"),
    definition(5, 1, {"WRITE SUCCESS"}),
    definition(4, 1, false, "RESERVED"),
    definition(0, 4, false, "DATA")
  });
  CAN_MSG_DEFINITION[0xC3] = pair<string, vector<definition>> ("ID_MCU_STATUS", {
    definition(5, 1, false, "STATE"),
    definition(4, 1, {"BMS OK HIGH", "IMD OK HIGH", "INVERTER POWERED", "SHUTDOWN B ABOVE THRESHOLD", "SHUTDOWN C ABOVE THRESHOLD", "SHUTDOWN D ABOVE THRESHOLD", "SHUTDOWN E ABOVE THRESHOLD", "SHUTDOWN F ABOVE THRESHOLD"}),
    definition(2, 2, true, "TEMPERATURE", "C", 0.01),
    definition(0, 2, false, "GLV BATTERY VOLTAGE", "V", 0.01)
  });
  CAN_MSG_DEFINITION[0xC4] = pair<string, vector<definition>> ("ID_MCU_PEDAL_READINGS", {
    definition(6, 2, false, "ACCELERATOR PEDAL RAW 1"),
    definition(4, 2, false, "ACCELERATOR PEDAL RAW 2"),
    definition(2, 2, false, "BRAKE PEDAL RAW"),
    definition(1, 1, {"ACCELERATOR IMPLAUSIBILITY", "BRAKE IMPLAUSIBILITY", "BRAKE PEDAL ACTIVE"}),
    definition(0, 1, false, "TORQUE MAP MODE" )
  });
  CAN_MSG_DEFINITION[0xCC] = pair<string, vector<definition>> ("ID_GLV_CURRENT_READINGS", {
    definition(2, 2, false, "ECU CURRENT", "A", 0.01),
    definition(0, 2, false, "COOLING CURRENT", "A", 0.01)
  });
  CAN_MSG_DEFINITION[0xD5] = pair<string, vector<definition>> ("ID_BMS_ONBOARD_TEMPERATURES", {
    definition(4, 2, true, "AVERAGE TEMPERATURE", "C", 0.01),
    definition(2, 2, true, "LOW TEMPERATURE", "C", 0.01),
    definition(0, 2, true, "HIGH TEMPERATURE", "C", 0.01)
  });
  CAN_MSG_DEFINITION[0xD6] = pair<string, vector<definition>> ("ID_BMS_ONBOARD_DETAILED_TEMPERATURES", {
    definition(4, 1, false, "IC ID"),
    definition(2, 2, true, "TEMPERATURE 0", "C", 0.01),
    definition(0, 2, true, "TEMPERATURE 1", "C", 0.01)
  });
  CAN_MSG_DEFINITION[0xD7] = pair<string, vector<definition>> ("ID_BMS_VOLTAGES", {
    definition(6, 2, false, "BMS VOLTAGE AVERAGE", "V", 0.0001),
    definition(4, 2, false, "BMS VOLTAGE LOW", "V", 0.0001),
    definition(2, 2, false, "BMS VOLTAGE HIGH", "V", 0.0001),
    definition(0, 2, false, "BMS VOLTAGE TOTAL", "V", 0.0001)
  });
  CAN_MSG_DEFINITION[0xD8] = pair<string, vector<definition>> ("ID_BMS_DETAILED_VOLTAGES", {
    definition(6, 1, false, "IC ID GROUP ID"),
    definition(4, 2, false, "VOLTAGE 0", "V", 0.001),
    definition(2, 2, false, "VOLTAGE 1", "V", 0.001),
    definition(0, 2, false, "VOLTAGE 2", "V", 0.001)
  });
  CAN_MSG_DEFINITION[0xD9] = pair<string, vector<definition>> ("ID_BMS_TEMPERATURES", {
    definition(4, 2, true, "BMS AVERAGE TEMPERATURE", "C", 0.01),
    definition(2, 2, true, "BMS LOW TEMPERATURE", "C", 0.01),
    definition(0, 2, true, "BMS HIGH TEMPERATURE", "C", 0.01)
  });
  CAN_MSG_DEFINITION[0xDA] = pair<string, vector<definition>> ("ID_DETAILED_TEMPERATURES", {
    definition(6, 1, false, "IC ID", "", 0.01),
    definition(4, 2, true, "TEMPERATURE 0", "C", 0.01),
    definition(2, 2, true, "TEMPERATURE 1", "C", 0.01),
    definition(0, 2, true, "TEMPERATURE 2", "C", 0.01)
  });
  CAN_MSG_DEFINITION[0xDB] = pair<string, vector<definition>> ("ID_BMS_STATUS", {
    definition(5, 1, false, "BMS STATE"),
    definition(3, 2, false, "BMS ERROR FLAGS"),
    definition(1, 2, true, "BMS CURRENT", "A", 0.01),
    definition(0, 1, false, "BMS FLAGS")
  });
  CAN_MSG_DEFINITION[0xDC] = pair<string, vector<definition>> ("ID_FH_WATCHDOG_TEST", {});
  CAN_MSG_DEFINITION[0xDD] = pair<string, vector<definition>> ("ID_CCU_STATUS", {
    definition(0, 1, {"CHARGER ENABLED"})
  });
  CAN_MSG_DEFINITION[0xDE] = pair<string, vector<definition>> ("ID_BMS_BALANCING_STATUS", {
    definition(0, 1, false, "BALANCING STATUS 0"), //what is this lol
    definition(1, 1, false, "BALANCING STATUS 1"),
    definition(2, 1, false, "BALANCING STATUS 2"),
    definition(3, 1, false, "BALANCING STATUS 3"),
    definition(4, 1, false, "BALANCING STATUS 4")
  });
  CAN_MSG_DEFINITION[0xDF] = pair<string, vector<definition>> ("ID_FCU_ACCELEROMETER", {});
  CAN_MSG_DEFINITION[0xE0] = pair<string, vector<definition>> ("ID_BMS_READ_WRITE_PARAMETER_COMMAND", {});
  CAN_MSG_DEFINITION[0xE1] = pair<string, vector<definition>> ("ID_BMS_PARAMETER_RESPONSE", {});
  CAN_MSG_DEFINITION[0xE2] = pair<string, vector<definition>> ("ID_BMS_COULOMB_COUNTS", {
    definition(4, 4, false, "TOTAL CHARGE", "C"),
    definition(0, 4, false, "TOTAL DISCHARGE", "C")
  });
  CAN_MSG_DEFINITION[0xE7] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_ALPHA", {
    definition(4, 4, true, "LATITUDE"),
    definition(0, 4, true, "LONGITUDE")
  });
  CAN_MSG_DEFINITION[0xE8] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_BETA", {
    definition(4, 4, true, "ALTITUDE"),
    definition(0, 4, true, "SPEED")
  });
  CAN_MSG_DEFINITION[0xE9] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_GAMMA", {
    definition(7, 1, false, "FIX QUALITY"),
    definition(6, 1, false, "SATELLITE COUNT"),
    definition(2, 4, true, "TIMESTAMP SECONDS"),
    definition(0, 2, true, "TIMESTAMP MILLISECONDS")
  });
}

#endif
