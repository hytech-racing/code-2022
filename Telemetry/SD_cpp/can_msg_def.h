#ifndef CAN_MSG_DEF_H
#define CAN_MSG_DEF_H

#ifdef USE_VECTOR_HEADER
  #include <vector.h>
#else
  #include <bits/stdc++.h>
#endif

using namespace std;

struct definition {
    int offset;
    int len;
    bool isSigned;
    string field;
    string units;
    vector<string> booleanMappings;

    definition(int o, int l, bool s, string f, string u = "") : offset(o), len(l), isSigned(s), field(f), units(u), booleanMappings(vector<string>()) {}
    definition(int o, int l, vector<string> m = {}) : offset(o), len(l), isSigned(false), field(""), units(""), booleanMappings(m) {}

    long long parse(unsigned long long u, int messageLen, vector<bool> &boolmap) {
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
    definition(0, 2, true, "MODULE A TEMP", "C"),
    definition(2, 2, true, "MODULE B TEMP", "C"),
    definition(4, 2, true, "MODULE C TEMP", "C"),
    definition(6, 2, true, "GATE DRIVER BOARD TEMP", "C")
  });
  CAN_MSG_DEFINITION[0xA1] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_2", {
    definition(0, 2, true, "CONTROL BOARD TEMPERATURE", "C"),
    definition(2, 2, true, "RTD 1 TEMPERATURE", "C"),
    definition(4, 2, true, "RTD 2 TEMPERATURE", "C"),
    definition(6, 2, true, "RTD 3 TEMPERATURE", "C")
  });
  CAN_MSG_DEFINITION[0xA2] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_3", {
    definition(0, 2, true, "RTD 4 TEMP", "C"),
    definition(2, 2, true, "RTD 5 TEMP", "C"),
    definition(4, 2, true, "MOTOR TEMP", "C"),
    definition(6, 2, true, "TORQUE SHUDDER", "C")
  });
  CAN_MSG_DEFINITION[0xA3] = pair<string, vector<definition>> ("ID_MC_ANALOG_INPUTS_VOLTAGES", {
    definition(0,2,true, "ANALOG INPUT 1"),
    definition(2,2,true, "ANALOG INPUT 2"),
    definition(4,2,true, "ANALOG INPUT 3"),
    definition(6,2,true, "ANALOG INPUT 4")
  });
  CAN_MSG_DEFINITION[0xA4] = pair<string, vector<definition>> ("ID_MC_DIGITAL_INPUT_STATUS", {
    definition(0, 1, {"DIGITAL INPUT 1", "DIGITAL INPUT 2", "DIGITAL INPUT 3", "DIGITAL INPUT 4", "DIGITAL INPUT 5", "DIGITAL INPUT 6", "DIGITAL INPUT 7", "DIGITAL INPUT 8"})
  });
  CAN_MSG_DEFINITION[0xA5] = pair<string, vector<definition>> ("ID_MC_MOTOR_POSITION_INFORMATION", {
    definition(0, 2, true, "MOTOR ANGLE"),
    definition(2, 2, true, "MOTOR SPEED", "RPM"),
    definition(4, 2, true, "ELEC OUTPUT FREQ"),
    definition(6, 2, true, "DELTA RESOLVER FILT")
  });
  CAN_MSG_DEFINITION[0xA6] = pair<string, vector<definition>> ("ID_MC_CURRENT_INFORMATION", {
    definition(0, 2, true, "PHASE A CURRENT", "A"),
    definition(2, 2, true, "PHASE B CURRENT", "A"),
    definition(4, 2, true, "PHASE C CURRENT", "A"),
    definition(6, 2, true, "DC BUS CURRENT", "A")
  });
  CAN_MSG_DEFINITION[0xA7] = pair<string, vector<definition>> ("ID_MC_VOLTAGE_INFORMATION", {
    definition(0, 2, true, "DC BUS VOLTAGE", "V"),
    definition(2, 2, true, "OUTPUT VOLTAGE", "V"),
    definition(4, 2, true, "PHASE AB VOLTAGE", "V"),
    definition(6, 2, true, "PHASE BC VOLTAGE", "V")
  });
  CAN_MSG_DEFINITION[0xA8] = pair<string, vector<definition>> ("ID_MC_FLUX_INFORMATION", {});
  CAN_MSG_DEFINITION[0xA9] = pair<string, vector<definition>> ("ID_MC_INTERNAL_VOLTAGES", {});
  CAN_MSG_DEFINITION[0xAA] = pair<string, vector<definition>> ("ID_MC_INTERNAL_STATES", {
    definition(0, 2, false, "VSM STATE"),
    definition(2, 1, false, "INVERTER STATE"),
    definition(3, 1, false, "INVERTER RUN MODE"),
    definition(4, 1, false, "INVERTER ACTIVE DISCHARGE STATE"),
    definition(5, 1, false, "INVERTER COMMAND MODE"),
    definition(6, 1, false, "INVERTER ENABLE"),
    definition(7, 1, false, "DIRECTION COMMAND")
  });
  CAN_MSG_DEFINITION[0xAB] = pair<string, vector<definition>> ("ID_MC_FAULT_CODES", {
    definition(0, 2, false, "POST FAULT LO"),
    definition(2, 2, false, "POST FAULT HI"),
    definition(4, 2, false, "RUN FAULT LO"),
    definition(6, 2, false, "RUN FAULT HI")
  });
  CAN_MSG_DEFINITION[0xAC] = pair<string, vector<definition>> ("ID_MC_TORQUE_TIMER_INFORMATION", {
    definition(0, 2, true, "COMMANDED TORQUE", "Nm"),
    definition(2, 2, true, "TORQUE FEEDBACK", "Nm"),
    definition(4, 4, false, "RMS UPTIME", "s")
  });
  CAN_MSG_DEFINITION[0xAD] = pair<string, vector<definition>> ("ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION", {
    definition(0, 2, false, "MODULATION INDEX"),
    definition(2, 2, true, "FLUX WEAKENING OUTPUT"),
    definition(4, 2, true, "ID COMMAND"),
    definition(6, 2, true, "IQ COMMAND")
  });
  CAN_MSG_DEFINITION[0xAE] = pair<string, vector<definition>> ("ID_MC_FIRMWARE_INFORMATION", {
    definition(0, 2, false, "eeprom_version_project_code"),
    definition(2, 2, false, "SOFTWARE VERSIO"),
    definition(4, 2, false, "DATE CODE MMDD"),
    definition(6, 2, false, "DATE CODE YYYY")
  });
  CAN_MSG_DEFINITION[0xC0] = pair<string, vector<definition>> ("ID_MC_COMMAND_MESSAGE", {
    definition(0, 2, true, "TORQUE COMMAND"),
    definition(2, 2, true, "ANGULAR VELOCITY"),
    definition(4, 1, {"DIRECTION"}),
    definition(5, 1, false, "INVERTER ENABLE DISCHARGE ENABLE"),
    definition(6, 2, true, "COMMANDED TORQUE LIST")
  });
  CAN_MSG_DEFINITION[0xC1] = pair<string, vector<definition>> ("ID_MC_READ_WRITE_PARAMETER_COMMAND", {
    definition(0, 2, false, "PARAMETER ADDRESS"),
    definition(2, 1, {"RW COMMAND"}),
    definition(3, 1, false, "RESERVED"),
    definition(4, 4, false, "DATA")
  });
  CAN_MSG_DEFINITION[0xC2] = pair<string, vector<definition>> ("ID_MC_READ_WRITE_PARAMETER_RESPONSE", {
    definition(0, 2, false, "PARAMETER ADDRESS"),
    definition(2, 1, {"WRITE SUCCESS"}),
    definition(3, 1, false, "RESERVED"),
    definition(4, 4, false, "DATA")
  });
  CAN_MSG_DEFINITION[0xC3] = pair<string, vector<definition>> ("ID_MCU_STATUS", {
    definition(0, 1, false, "sTATE"),
    definition(1, 1, {"BMS OK HIGH", "IMD OKHS HIGH", "INVERTER POWERED", "SHUTDOWN B ABOVE THRESHOLD", "SHUTDOWN C ABOVE THRESHOLD", "SHUTDOWN D ABOVE THRESHOLD", "SHUTDOWN E ABOVE THRESHOLD", "SHUTDOWN F ABOVE THRESHOLD"}),
    definition(3, 2, true, "TEMPERATURE"),
    definition(5, 2, false, "GLV BATTERY VOLTAGE")
  });
  CAN_MSG_DEFINITION[0xC4] = pair<string, vector<definition>> ("ID_MCU_PEDAL_READINGS", {
    definition(0, 2, false, "ACCELERATOR PEDAL RAW 1"),
    definition(2, 2, false, "ACCELERATOR PEDAL RAW 2"),
    definition(4, 2, false, "BRAKE PEDAL RAW"),
    definition(6, 1, {"ACCELERATOR IMPLAUSIBILITY", "BRAKE IMPLAUSIBILITY", "BRAKE PEDAL ACTIVE"}),
    definition(7, 1, false, "TORQUE MAP MODE" )
  });
  CAN_MSG_DEFINITION[0xCC] = pair<string, vector<definition>> ("ID_GLV_CURRENT_READINGS", {});
  CAN_MSG_DEFINITION[0xD7] = pair<string, vector<definition>> ("ID_BMS_VOLTAGES", {
    definition(0, 2, false, "BMS VOLTAGE AVERAGE", "V"),
    definition(2, 2, true, "BMS VOLTAGE LOW", "V"),
    definition(4, 2, true, "BMS VOLTAGE HIGH", "V"),
    definition(6, 2, true, "BMS VOLTAGE TOTAL", "V")
  });
  CAN_MSG_DEFINITION[0xD8] = pair<string, vector<definition>> ("ID_BMS_DETAILED_VOLTAGES", {
    definition(0, 1, true, "IC ID GROUP ID"),
    definition(1, 2, true, "VOLTAGE 0"),
    definition(3, 2, true, "VOLTAGE 1"),
    definition(5, 2, true, "VOLTAGE 2")
  });
  CAN_MSG_DEFINITION[0xD9] = pair<string, vector<definition>> ("ID_BMS_TEMPERATURES", {
    definition(0, 2, true, "BMS AVERAGE TEMPERATURE", "C"),
    definition(2, 2, true, "BMS LOW TEMPERATURE", "C"),
    definition(4, 2, true, "BMS HIGH TEMPERATURE", "C")
  });
  CAN_MSG_DEFINITION[0xDA] = pair<string, vector<definition>> ("ID_DETAILED_TEMPERATURES", {
    definition(0, 1, false, "IC ID"),
    definition(1, 2, true, "TEMPERATURE 0", "C"),
    definition(3, 2, true, "TEMPERATURE 1", "C"),
    definition(5, 2, true, "TEMPERATURE 2", "C")
  });
  CAN_MSG_DEFINITION[0xDB] = pair<string, vector<definition>> ("ID_BMS_STATUS", {
    definition(0, 1, false, "BMS STATE"),
    definition(1, 2, false, "BMS ERROR FLAGS"),
    definition(3, 2, true, "BMS CURRENT"),
    definition(5, 1, false, "BMS FLAGS")
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
    definition(0, 4, true, "TOTAL CHARGE", "C"),
    definition(4, 4, true, "TOTAL DISCHARGE", "C")
  });
  CAN_MSG_DEFINITION[0xE7] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_ALPHA", {});
  CAN_MSG_DEFINITION[0xE8] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_GAMMA", {});
}

#endif
