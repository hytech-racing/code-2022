#ifndef CAN_MSG_DEF_H
#define CAN_MSG_DEF_H

#include <vector.h>
using namespace std;

struct definition {
    int offset;
    int len;
    bool isSigned;
    string field;
    string description;
    vector<string> booleanMappings;

    definition(int o, int l, bool s, string f, string d = "", vector<string> m = {}) : offset(o), len(l), isSigned(s), field(f), description(d), booleanMappings(m) {}

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
                // cout << bitset<64>(rawData) << endl << bitset<64>(mask) << endl << endl;
                // rawData = (rawData ^ mask) + 1;
                // return -rawData;
            }
        }
        return rawData;
    }
};

static vector<pair<string, vector<definition>>> CAN_MSG_DEFINITION (0x100, pair<string, vector<definition>>("ERROR: NO DEFINITION FOUND", {}));

static void loadLookupTable() {
  vector<definition> empty;
  CAN_MSG_DEFINITION[0xA0] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_1", {
    definition(0, 2, true, "module_a_temperature"),
    definition(2, 2, true, "module_b_temperature"),
    definition(4, 2, true, "module_c_temperature"),
    definition(6, 2, true, "gate_driver_board_temperature")
  });
  CAN_MSG_DEFINITION[0xA1] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_2", {
    definition(0, 2, true, "control_board_temperature"),
    definition(2, 2, true, "rtd_1_temperature"),
    definition(4, 2, true, "rtd_2_temperature"),
    definition(6, 2, true, "rtd_3_temperature")
  });
  CAN_MSG_DEFINITION[0xA2] = pair<string, vector<definition>> ("ID_MC_TEMPERATURES_3", {
    definition(0, 2, true, "rtd_4_temperature"),
    definition(2, 2, true, "rtd_5_temperature"),
    definition(4, 2, true, "motor_temperature"),
    definition(6, 2, true, "torque_shudder")
  });
  CAN_MSG_DEFINITION[0xA3] = pair<string, vector<definition>> ("ID_MC_ANALOG_INPUTS_VOLTAGES", {
    definition(0,2,true, "analog_input_1"),
    definition(2,2,true, "analog_input_2"),
    definition(4,2,true, "analog_input_3"),
    definition(6,2,true, "analog_input_4")
  });
  CAN_MSG_DEFINITION[0xA4] = pair<string, vector<definition>> ("ID_MC_DIGITAL_INPUT_STATUS", {
    definition(0, 1, false, "", "", {"digital_input_1", "digital_input_2", "digital_input_3", "digital_input_4", "digital_input_5", "digital_input_6", "digital_input_7", "digital_input_8"})
  });
  CAN_MSG_DEFINITION[0xA5] = pair<string, vector<definition>> ("ID_MC_MOTOR_POSITION_INFORMATION", {
    definition(0, 2, true, "motor_angle"),
    definition(2, 2, true, "motor_speed"),
    definition(4, 2, true, "electrical_output_frequency"),
    definition(6, 2, true, "delta_resolver_filtered")
  });
  CAN_MSG_DEFINITION[0xA6] = pair<string, vector<definition>> ("ID_MC_CURRENT_INFORMATION", {
    definition(0, 2, true, "phase_a_current"),
    definition(2, 2, true, "phase_b_current"),
    definition(4, 2, true, "phase_c_current"),
    definition(6, 2, true, "dc_bus_current")
  });
  CAN_MSG_DEFINITION[0xA7] = pair<string, vector<definition>> ("ID_MC_VOLTAGE_INFORMATION", {
    definition(0, 2, true, "dc_bus_voltage"),
    definition(2, 2, true, "output_voltage"),
    definition(4, 2, true, "phase_ab_voltage"),
    definition(6, 2, true, "phase_bc_voltage")
  });
  CAN_MSG_DEFINITION[0xA8] = pair<string, vector<definition>> ("ID_MC_FLUX_INFORMATION", {
  });
  CAN_MSG_DEFINITION[0xA9] = pair<string, vector<definition>> ("ID_MC_INTERNAL_VOLTAGES", {
  });
  CAN_MSG_DEFINITION[0xAA] = pair<string, vector<definition>> ("ID_MC_INTERNAL_STATES", {
    definition(0, 2, false, "vsm_state"),
    definition(2, 1, false, "inverter_state"),
    definition(3, 1, false, "relay_state"),
    definition(4, 1, false, "inverter_run_mode_discharge_state"),
    definition(5, 1, false, "inverter_command_mode"),
    definition(6, 1, false, "inverter_enable"),
    definition(7, 1, false, "direction_command")
  });
  CAN_MSG_DEFINITION[0xAB] = pair<string, vector<definition>> ("ID_MC_FAULT_CODES", {
    definition(0, 2, false, "post_fault_lo"),
    definition(2, 2, false, "post_fault_hi"),
    definition(4, 2, false, "run_fault_lo"),
    definition(6, 2, false, "run_fault_hi")
  });
  CAN_MSG_DEFINITION[0xAC] = pair<string, vector<definition>> ("ID_MC_TORQUE_TIMER_INFORMATION", {
    definition(0, 2, true, "commanded_torque"),
    definition(2, 2, true, "torque_feedback"),
    definition(4, 4, false, "power_on_timer")
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
    definition(2, 2, true, "angular_velocity"),
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
    definition(0, 1, false, "state", "ECU State"),
    definition(1, 1, false, "flags", "ECU Flags (packed booleans)", {"bms_ok_high", "imd_okhs_high", "inverter_powered", "shutdown_b_above_threshold", "shutdown_c_above_threshold", "shutdown_d_above_threshold", "shutdown_e_above_threshold", "shutdown_f_above_threshold"}),
    definition(3, 2, true, "temperature", "Temperature of the board (in ºC) times 100"),
    definition(5, 2, false, "glv_battery_voltage", "GLV battery voltage reading (in Volts) times 1000")
  });
  CAN_MSG_DEFINITION[0xC4] = pair<string, vector<definition>> ("ID_MCU_PEDAL_READINGS", {
    definition(0, 2, false, "accelerator_pedal_raw_1", "Accelerator Pedal 1 Raw ADC Reading"),
    definition(2, 2, false, "accelerator_pedal_raw_2", "Accelerator Pedal 2 Raw ADC Reading"),
    definition(4, 2, false, "brake_pedal_raw", "Brake Pedal Raw ADC Reading"),
    definition(6, 1, false, "pedal_flags", "Pedal Error Flags (packed booleans)",	{"accelerator_implausibility", "brake_implausibility", "brake_pedal_active"}),
    definition(7, 1, false, "torque_map_mode", "Torque map mode in use (pedal mapping, regen, etc)")
  });
  CAN_MSG_DEFINITION[0xCC] = pair<string, vector<definition>> ("ID_GLV_CURRENT_READINGS", {
  });
  CAN_MSG_DEFINITION[0xD7] = pair<string, vector<definition>> ("ID_BMS_VOLTAGES", {
    definition(0, 2, false, "average_voltage"),
    definition(2, 2, true, "low_voltage"),
    definition(4, 2, true, "high_voltage"),
    definition(6, 2, true, "total_voltage")
  });
  CAN_MSG_DEFINITION[0xD8] = pair<string, vector<definition>> ("ID_BMS_DETAILED_VOLTAGES", {
    definition(0, 1, true, "ic_id_group_id"),
    definition(1, 2, true, "voltage_0"),
    definition(3, 2, true, "voltage_1"),
    definition(5, 2, true, "voltage_2")
  });
  CAN_MSG_DEFINITION[0xD9] = pair<string, vector<definition>> ("ID_BMS_TEMPERATURES", {
    definition(0, 2, true, "average_temperature"),
    definition(2, 2, true, "low_temperature"),
    definition(4, 2, true, "high_temperature")
  });
  CAN_MSG_DEFINITION[0xDA] = pair<string, vector<definition>> ("ID_DETAILED_TEMPERATURES", {
    definition(0, 1, false, "ic_id"),
    definition(1, 2, true, "temperature_0"),
    definition(3, 2, true, "temperature_1"),
    definition(5, 2, true, "temperature_2")
  });
  CAN_MSG_DEFINITION[0xDB] = pair<string, vector<definition>> ("ID_BMS_STATUS", {
    definition(0, 1, false, "state"),
    definition(1, 2, false, "error_flags"),
    definition(3, 2, true, "current"),
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
  CAN_MSG_DEFINITION[0xE0] = pair<string, vector<definition>> ("ID_BMS_READ_WRITE_PARAMETER_COMMAND", empty);
  CAN_MSG_DEFINITION[0xE1] = pair<string, vector<definition>> ("ID_BMS_PARAMETER_RESPONSE", empty);
  CAN_MSG_DEFINITION[0xE2] = pair<string, vector<definition>> ("ID_BMS_COULOMB_COUNTS", {
    definition(0, 4, true, "total_charge"),
    definition(4, 4, true, "total_discharge")
  });
  CAN_MSG_DEFINITION[0xE7] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_ALPHA", empty);
  CAN_MSG_DEFINITION[0xE8] = pair<string, vector<definition>> ("ID_MCU_GPS_READINGS_GAMMA", empty);
}

#endif
