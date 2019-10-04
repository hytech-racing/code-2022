#ifndef CAN_MSG_DEF_H
#define CAN_MSG_DEF_H

#include <bits/stdc++.h>
using namespace std;

struct definition {
    int offset;
    int len;
    bool isSigned;
    string field;
    vector<string> booleanMappings;

    definition(int o, int l, bool s, string f, vector<string> m = {}) : offset(o), len(l), isSigned(s), field(f), booleanMappings(m) {}

    int parse(unsigned long long u, int messageLen, vector<bool> &boolmap) {
        unsigned long long mask = (1ULL << (8 * len)) - 1;
        int rawData = (u >> (8*(messageLen-offset-len))) & mask;

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
  vector<definition> empty;
  CAN_MSG_DEFINITION[0xA0] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_1", {
    definition(0, 2, true, "MODULE A TEMP"), //C
    definition(2, 2, true, "MODULE B TEMP"), //C
    definition(4, 2, true, "MODULE C TEMP"), //C
    definition(6, 2, true, "GATE DRIVER BOARD TEMP") //C
  });
  CAN_MSG_DEFINITION[0xA1] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_2", {
    definition(0, 2, true, "control_board_temperature"), //degrees Celsius
    definition(2, 2, true, "rtd_1_temperature"), //degrees Celsius
    definition(4, 2, true, "rtd_2_temperature"), //degrees Celsius
    definition(6, 2, true, "rtd_3_temperature") //degrees Celsius
  });
  CAN_MSG_DEFINITION[0xA2] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_3", {
    definition(0, 2, true, "RTD 4 TEMP"), //C
    definition(2, 2, true, "RTD 5 TEMP"), //C
    definition(4, 2, true, "MOTOR TEMP"), //C
    definition(6, 2, true, "TORQUE SHUDDER") // Nm
  });
  CAN_MSG_DEFINITION[0xA3] = pair<string, vector<definition>> ("ID_MC_ANALOG_INPUTS_VOLTAGES", {
    definition(0,2,true, "analog_input_1"),
    definition(2,2,true, "analog_input_2"),
    definition(4,2,true, "analog_input_3"),
    definition(6,2,true, "analog_input_4")
  });
  CAN_MSG_DEFINITION[0xA4] = pair<string, vector<definition>> ("ID_MC_DIGITAL_INPUT_STATUS", {
    definition(0, 1, false, "", {"digital_input_1", "digital_input_2", "digital_input_3", "digital_input_4", "digital_input_5", "digital_input_6", "digital_input_7", "digital_input_8"})
  });
  CAN_MSG_DEFINITION[0xA5] = pair<string, vector<definition>> ("ID_MC_MOTOR_POSITION_INFORMATION", {
    definition(0, 2, true, "MOTOR ANGLE"),
    definition(2, 2, true, "MOTOR SPEED"), // RPM
    definition(4, 2, true, "ELEC OUTPUT FREQ"),
    definition(6, 2, true, "DELTA RESOLVER FILT")
  });
  CAN_MSG_DEFINITION[0xA6] = pair<string, vector<definition>> ("ID_MC_CURRENT_INFORMATION", {
    definition(0, 2, true, "PHASE A CURRENT"), //A
    definition(2, 2, true, "PHASE B CURRENT"), //A
    definition(4, 2, true, "PHASE C CURRENT"), //A
    definition(6, 2, true, "DC BUS CURRENT")  //A
  });
  CAN_MSG_DEFINITION[0xA7] = pair<string, vector<definition>> ("ID_MC_VOLTAGE_INFORMATION", {
    definition(0, 2, true, "DC BUS VOLTAGE"), //V
    definition(2, 2, true, "OUTPUT VOLTAGE"), //V
    definition(4, 2, true, "PHASE AB VOLTAGE"), //V
    definition(6, 2, true, "PHASE BC VOLTAGE") //V
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
    definition(0, 2, false, "POST FAULT LO"), //hex
    definition(2, 2, false, "POST FAULT HI"), //hex
    definition(4, 2, false, "RUN FAULT LO"), //hex
    definition(6, 2, false, "RUN FAULT HI") //hex
  });
  CAN_MSG_DEFINITION[0xAC] = pair<string, vector<definition>> ("ID_MC_TORQUE_TIMER_INFORMATION", {
    definition(0, 2, true, "COMMANDED TORQUE"), //Nm
    definition(2, 2, true, "TORQUE FEEDBACK"), //Nm
    definition(4, 4, false, "RMS UPTIME") //s
  });
  CAN_MSG_DEFINITION[0xAD] = pair<string, vector<definition>> ("ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION", {
    definition(0, 2, false, "modulation_index"),
    definition(2, 2, true, "flux_weakening_output"),
    definition(4, 2, true, "id_command"),
    definition(6, 2, true, "iq_command")
  });
  CAN_MSG_DEFINITION[0xAE] = pair<string, vector<definition>> ("ID_MC_FIRMWARE_INFORMATION", {
    definition(0, 2, false, "eeprom_version_project_code"),
    definition(2, 2, false, "software_version"),
    definition(4, 2, false, "date_code_mmdd"),
    definition(6, 2, false, "date_code_yyyy")
  });
  CAN_MSG_DEFINITION[0xC0] = pair<string, vector<definition>> ("ID_MC_COMMAND_MESSAGE", {
    definition(0, 2, true, "torque_command"),
    definition(2, 2, true, "angular_velocity"), //radians/second
    definition(4, 1, false, {"direction"}),
    definition(5, 1, false, "inverter_enable_discharge_enable"),
    definition(6, 2, true, "commanded_torque_limit")
  });
  CAN_MSG_DEFINITION[0xC1] = pair<string, vector<definition>> ("ID_MC_READ_WRITE_PARAMETER_COMMAND", {
    definition(0, 2, false, "parameter_address"),
    definition(2, 1, false, {"rw_command"}),
    definition(3, 1, false, "reserved"),
    definition(4, 4, false, "data")
  });
  CAN_MSG_DEFINITION[0xC2] = pair<string, vector<definition>> ("ID_MC_READ_WRITE_PARAMETER_RESPONSE", {
    definition(0, 2, false, "parameter_address"),
    definition(2, 1, false, {"write_success"}),
    definition(3, 1, false, "reserved"),
    definition(4, 4, false, "data")
  });
  CAN_MSG_DEFINITION[0xC3] = pair<string, vector<definition>> ("ID_MCU_STATUS", {
    definition(0, 1, false, "state"),
    definition(1, 1, false, "flags", {"bms_ok_high", "imd_okhs_high", "inverter_powered", "shutdown_b_above_threshold", "shutdown_c_above_threshold", "shutdown_d_above_threshold", "shutdown_e_above_threshold", "shutdown_f_above_threshold"}),
    definition(3, 2, true, "temperature"),
    definition(5, 2, false, "glv_battery_voltage")
  });
  CAN_MSG_DEFINITION[0xC4] = pair<string, vector<definition>> ("ID_MCU_PEDAL_READINGS", {
    definition(0, 2, false, "accelerator_pedal_raw_1"),
    definition(2, 2, false, "accelerator_pedal_raw_2"),
    definition(4, 2, false, "brake_pedal_raw"),
    definition(6, 1, false, "pedal_flags",	{"accelerator_implausibility", "brake_implausibility", "brake_pedal_active"}),
    definition(7, 1, false, "torque_map_mode" )
  });
  CAN_MSG_DEFINITION[0xCC] = pair<string, vector<definition>> ("ID_GLV_CURRENT_READINGS", {});
  CAN_MSG_DEFINITION[0xD7] = pair<string, vector<definition>> ("ID_BMS_VOLTAGES", {
    definition(0, 2, false, "BMS VOLTAGE AVERAGE"), //V
    definition(2, 2, true, "BMS VOLTAGE LOW"), //V
    definition(4, 2, true, "BMS VOLTAGE HIGH"), //V
    definition(6, 2, true, "BMS VOLTAGE TOTAL") //V
  });
  CAN_MSG_DEFINITION[0xD8] = pair<string, vector<definition>> ("ID_BMS_DETAILED_VOLTAGES", {
    definition(0, 1, true, "ic_id_group_id"),
    definition(1, 2, true, "voltage_0"), //Volts
    definition(3, 2, true, "voltage_1"), //Volts
    definition(5, 2, true, "voltage_2") //Volts
  });
  CAN_MSG_DEFINITION[0xD9] = pair<string, vector<definition>> ("ID_BMS_TEMPERATURES", {
    definition(0, 2, true, "BMS AVERAGE TEMPERATURE"), //C
    definition(2, 2, true, "BMS LOW TEMPERATURE"), //C
    definition(4, 2, true, "BMS HIGH TEMPERATURE") //C
  });
  CAN_MSG_DEFINITION[0xDA] = pair<string, vector<definition>> ("ID_DETAILED_TEMPERATURES", {
    definition(0, 1, false, "ic_id"),
    definition(1, 2, true, "temperature_0"), //degrees Celsius
    definition(3, 2, true, "temperature_1"), //degrees Celsius
    definition(5, 2, true, "temperature_2") //degrees Celsius
  });
  CAN_MSG_DEFINITION[0xDB] = pair<string, vector<definition>> ("ID_BMS_STATUS", {
    definition(0, 1, false, "state"),
    definition(1, 2, false, "error_flags"),
    definition(3, 2, true, "current"), //amperes
    definition(5, 1, false, "flags")
  });
  CAN_MSG_DEFINITION[0xDC] = pair<string, vector<definition>> ("ID_FH_WATCHDOG_TEST", {
  });
  CAN_MSG_DEFINITION[0xDD] = pair<string, vector<definition>> ("ID_CCU_STATUS", {
    definition(0, 1, false, {"charger_enabled"})
  });
  CAN_MSG_DEFINITION[0xDE] = pair<string, vector<definition>> ("ID_BMS_BALANCING_STATUS", {
    definition(0, 1, false, "balancing_status[0]"),
    definition(1, 1, false, "balancing_status[1]"),
    definition(2, 1, false, "balancing_status[2]"),
    definition(3, 1, false, "balancing_status[3]"),
    definition(4, 1, false, "balancing_status[4]")
  });
  CAN_MSG_DEFINITION[0xDF] = pair<string, vector<definition>> ("ID_FCU_ACCELEROMETER", {
  });
  CAN_MSG_DEFINITION[0xE0] = pair<string, vector<definition>> ("ID_BMS_READ_WRITE_PARAMETER_COMMAND", {});
  CAN_MSG_DEFINITION[0xE1] = pair<string, vector<definition>> ("ID_BMS_PARAMETER_RESPONSE", {});
  CAN_MSG_DEFINITION[0xE2] = pair<string, vector<definition>> ("ID_BMS_COULOMB_COUNTS", {
    definition(0, 4, true, "total_charge"), //coulombs
    definition(4, 4, true, "total_discharge") //coulombs
  });
  CAN_MSG_DEFINITION[0xE7] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_ALPHA", {});
  CAN_MSG_DEFINITION[0xE8] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_GAMMA", {});
}

#endif
