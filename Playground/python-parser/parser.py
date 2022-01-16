"""
@Author: Bo Han Zhu
@Date: 1/15/2022
@Description: HyTech custom python parser. Reads CSVs from Raw_Data, parses them, and writes to Parsed_Data
@TODO: Also output to MATLAB struct

parse_folder --> parse_file --> parse_time
                            --> parse_message --> parse_ID_XXXXXXXXX
"""

# Imports
from optparse import Values
import os
import sys
from datetime import datetime

DEBUG = False # Set True for option error print statements

def hex_to_decimal(hex, bits, is_signed):
    """
    @brief: Helper function to convert a hexadecimal to a decimal. First swaps endianness and then performs twos-complement if needed.
            Referenced partially from: https://stackoverflow.com/questions/6727875/hex-string-to-signed-int-in-python-3-2
    @input: A string representing the hexadecimal number, how many bits it has, and whether if it is signed or unsigned (True --> signed)
    @return: The corresponding decimal value as an integer.
    """

    # Swaps to small-endian
    value = ""
    for i in range(bits // 4):
        if i % 2 == 0:
            value = hex[i:i+2] + value
    value = int(value, 16)

    # Checks if needed to perform twos-complement or not; if yes, then performs twos-complement
    if is_signed and value & (1 << (bits - 1)):
        value -= 1 << bits

    return value


########################################################################
# Custom Parsing Functions Begin
########################################################################
'''
@brief: Each one of these functions parses the message depending on the corresponding header file in code-2022/Libraries/HyTech_CAN.
        Must be updated consistently when changes occur in the HyTech Library.
@input: A string of a hexadecimal raw message
@return: A four-element list [message, label[], value[], unit[]] 
'''

def parse_ID_MC_TEMPERATURES1(raw_message):
    message = "MC_temperatures_1"
    labels = ["module_a_temperature", "module_b_temperature", "module_c_temperature", "gate_driver_board_temperature"]
    values = [hex_to_decimal(raw_message[0:4], 16, True) / 10.0, hex_to_decimal(raw_message[4:8], 16, True) / 10.0, hex_to_decimal(raw_message[8:12], 16, True) / 10.0, hex_to_decimal(raw_message[12:16], 16, True) / 10.0]
    units = ["C", "C", "C", "C"]
    return [message, labels, values, units]

def parse_ID_MC_TEMPERATURES2(raw_message):
    message = "MC_temperatures_2"
    labels = ["control_board_temperature", "rtd_1_temperature", "rtd_2_temperature", "rtd_3_temperature"]
    values = [hex_to_decimal(raw_message[0:4], 16, True) / 10.0, hex_to_decimal(raw_message[4:8], 16, True) / 10.0, hex_to_decimal(raw_message[8:12], 16, True) / 10.0, hex_to_decimal(raw_message[12:16], 16, True) / 10.0]
    units = ["C", "C", "C", "C"]
    return [message, labels, values, units]

def parse_ID_MC_TEMPERATURES3(raw_message):
    message = "MC_temperatures_3"
    labels = ["rtd_4_temperatures", "rtd_5_temperature", "motor_temperature", "torque_shudder"]
    values = [hex_to_decimal(raw_message[0:4], 16, True) / 10.0, hex_to_decimal(raw_message[4:8], 16, True) / 10.0, hex_to_decimal(raw_message[8:12], 16, True) / 10.0, hex_to_decimal(raw_message[12:16], 16, True) / 10.0]
    units = ["C", "C", "C", "N-m"]
    return [message, labels, values, units]

def parse_ID_MC_ANALOG_INPUTS_VOLTAGES(raw_message):
    message = "MC_analog_input_voltages"
    labels = ["MC_analog_input_1", "MC_analog_input_2", "MC_analog_input_3", "MC_analog_input_4"]
    values = [hex_to_decimal(raw_message[0:4], 16, True), hex_to_decimal(raw_message[4:8], 16, True), hex_to_decimal(raw_message[8:12], 16, True), hex_to_decimal(raw_message[12:16], 16, True)]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_DIGITAL_INPUTS_STATUS(raw_message):
    message = "MC_digital_input_status"
    labels = ["MC_digital_input_1", "MC_digital_input_2", "MC_digital_input_3", "MC_digital_input_4", "MC_digital_input_5", "MC_digital_input_6", "MC_digital_input_7", "MC_digital_input_8"]
    values = [raw_message[1], raw_message[3], raw_message[5], raw_message[7], raw_message[9], raw_message[11], raw_message[13], raw_message[15]]
    units = ["", "", "", "", "", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_MOTOR_POSITION_INFORMATION(raw_message):
    message = "MC_motor_position_information"
    labels = ["motor_angle", "motor_speed", "elec_output_freq", "delta_resolver_filtered"]
    values = [hex_to_decimal(raw_message[0:4], 16, True) / 10.0, hex_to_decimal(raw_message[4:8], 16, True), hex_to_decimal(raw_message[8:12], 16, True) / 10.0, hex_to_decimal(raw_message[12:16], 16, True)]
    units = ["", "RPM", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_CURRENT_INFORMATION(raw_message):
    message = "MC_current_information"
    labels = ["phase_a_current", "phase_b_current", "phase_c_current", "dc_bus_current"]
    values = [hex_to_decimal(raw_message[0:4], 16, True) / 10.0, hex_to_decimal(raw_message[4:8], 16, True) / 10.0, hex_to_decimal(raw_message[8:12], 16, True) / 10.0, hex_to_decimal(raw_message[12:16], 16, True) / 10.0]
    units = ["A", "A", "A", "A"]
    return [message, labels, values, units]

def parse_ID_MC_VOLTAGE_INFORMATION(raw_message):
    message = "MC_voltage_information"
    labels = ["dc_bus_voltage", "output_voltage", "phase_ab_voltage", "phase_bc_voltage"]
    values = [hex_to_decimal(raw_message[0:4], 16, True) / 10.0, hex_to_decimal(raw_message[4:8], 16, True) / 10.0, hex_to_decimal(raw_message[8:12], 16, True) / 10.0, hex_to_decimal(raw_message[12:16], 16, True) / 10.0]
    units = ["V", "V", "V", "V"]
    return [message, labels, values, units]

def parse_ID_MC_FLUX_INFORMATION(raw_message):
    if DEBUG: print("ERROR: Do not know how to parse CAN ID 0xA8. This CAN ID does not exist in the HyTech CAN Library.")
    return ["MC_flux_information", ["UNPARSEABLE"], ["UNPARSEABLE"], ["UNPARSEABLE"]]
    
def parse_ID_MC_INTERNAL_VOLTAGES(raw_message):
    if DEBUG: print("ERROR: Do not know how to parse CAN ID 0xA9. This CAN ID does not exist in the HyTech CAN Library.")
    return ["MC_flux_information", ["UNPARSEABLE"], ["UNPARSEABLE"], ["UNPARSEABLE"]]

def parse_ID_MC_INTERNAL_STATES(raw_message):
    message = "MC_internal_states"
    labels = ["vsm_state", "inverter_state", "relay_active_1", "relay_active_2", "relay_active_3", "relay_active_4", "relay_active_5", "relay_active_6", "inverter_run_mode", "inverter_active_discharge_state", "inverter_command_mode", "inverter_enable_state", "inverter_enable_lockout", "direction_command"]
    
    relay_state = hex_to_decimal(raw_message[6:8], 8, False)
    relay_state_1 = str(relay_state & 0x01)
    relay_state_2 = str((relay_state & 0x02) >> 1)
    relay_state_3 = str((relay_state & 0x04) >> 2)
    relay_state_4 = str((relay_state & 0x08) >> 3)
    relay_state_5 = str((relay_state & 0x10) >> 4)
    relay_state_6 = str((relay_state & 0x20) >> 5)
    inverter_run_mode_discharge_state = hex_to_decimal(raw_message[8:10], 8, False)
    inverter_run_mode = str(inverter_run_mode_discharge_state & 1)
    inverter_active_discharge_status = str(inverter_run_mode_discharge_state >> 5)
    inverter_enable = hex_to_decimal(raw_message[12:14], 8, False)
    inverter_enable_state = str(inverter_enable & 1)
    inverter_enable_lockout = str((inverter_enable & 0x80) >> 7)

    values = ["0x" + raw_message[0:4], "0x" + raw_message[4:6], relay_state_1, relay_state_2, relay_state_3, relay_state_4, relay_state_5, relay_state_6, inverter_run_mode, inverter_active_discharge_status, "0x" + raw_message[10:12], inverter_enable_state, inverter_enable_lockout, "0x" + raw_message[14:16]]
    units = ["", "", "", "", "", "", "", "", "", "", "", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_FAULT_CODES(raw_message):
    message = "MC_fault_codes"
    labels = [
        "post_fault_lo",
        "post_lo_hw_gate_desaturation_fault",
        "post_lo_hw_overcurrent_fault",
        "post_lo_accelerator_shorted",
        "post_lo_accelerator_opened",
        "post_lo_current_sensor_low",
        "post_lo_current_sensor_high",
        "post_lo_module_temperature_low",
        "post_lo_module_temperature_high",
        "post_lo_ctrl_pcb_temperature_low",
        "post_lo_ctrl_pcb_temperature_high",
        "post_lo_gate_drive_pcb_temperature_low",
        "post_lo_gate_drive_temperature_high",
        "post_lo_5v_sense_voltage_low",
        "post_lo_5v_sense_voltage_high",
        "post_lo_12v_sense_voltage_low",
        "post_lo_12v_sense_voltage_high",
        "post_fault_hi",
        "post_hi_25v_sense_voltage_low",
        "post_hi_25v_sense_voltage_high",
        "post_hi_15v_sense_voltage_low",
        "post_hi_15v_sense_voltage_high",
        "post_hi_dc_bus_voltage_high",
        "post_hi_dc_bus_voltage_low",
        "post_hi_precharge_timeout",
        "post_hi_precharge_voltage_failure",
        "post_hi_eeprom_checksum_invalidate",
        "post_hi_eeproom_data_out_of_range",
        "post_hi_eeprom_update_required",
        "post_hi_reserved1",
        "post_high_reserved2",
        "post_high_reserved3",
        "post_hi_brake_shorted",
        "post_hi_brake_open",
        "run_fault_lo",
        "run_lo_motor_overspeed_fault",
        "run_lo_overcurrent_fault",
        "run_lo_overvoltage_fault", 
        "run_lo_inverter_overtemperature_fault",
        "run_lo_accelerator_input_shorted_fault",
        "run_lo_accelerator_input_open_fault",
        "run_lo_direction_command_fault",
        "run_lo_inverter_response_timeout_fault",
        "run_lo_hardware_gatedesaturation_fault",
        "run_lo_hardware_overcurrent_fault",
        "run_lo_undervoltage_fault",
        "run_lo_can_command_message_lost_fault",
        "run_lo_motor_overtemperature_fault",
        "run_lo_reserved1",
        "run_lo_reserved2",
        "run_lo_reserved3",
        "run_fault_hi",
        "run_hi_brake_input_shorted_fault", 
        "run_hi_brake_input_open_fault",
        "run_hi_module_a_overtemperature_fault",
        "run_hi_module_b_overtemperature_fault", 
        "run_hi_module_c_overtemperature_fault",
        "run_hi_pcb_overtemperature_fault",
        "run_hi_gate_drive_board_1_overtemperature_fault",
        "run_hi_gate_drive_board_2_overtemperature_fault",
        "run_hi_gate_drive_board_3_overtemperature_fault",
        "run_hi_current_sensor_fault",
        "run_hi_reserved1",
        "run_hi_reserved2",
        "run_hi_reserved3",
        "run_hi_reserved4", 
        "run_hi_resolver_not_connected", 
        "run_hi_inverter_discharge_active"
    ]

    post_fault_lo = hex_to_decimal(raw_message[0:4], 16, False)
    post_fault_hi = hex_to_decimal(raw_message[4:8], 16, False)
    run_fault_lo = hex_to_decimal(raw_message[8:12], 16, False)
    run_fault_hi = hex_to_decimal(raw_message[12:16], 16, False)

    values = [
        "0x" + raw_message[0:4],
        str(post_fault_lo & 0x0001),
        str((post_fault_lo & 0x0002) >> 1),
        str((post_fault_lo & 0x0004) >> 2),
        str((post_fault_lo & 0x0008) >> 3),
        str((post_fault_lo & 0x0010) >> 4),
        str((post_fault_lo & 0x0020) >> 5),
        str((post_fault_lo & 0x0040) >> 6),
        str((post_fault_lo & 0x0080) >> 7),
        str((post_fault_lo & 0x0100) >> 8),
        str((post_fault_lo & 0x0200) >> 9),
        str((post_fault_lo & 0x0400) >> 10),
        str((post_fault_lo & 0x0800) >> 11),
        str((post_fault_lo & 0x1000) >> 12),
        str((post_fault_lo & 0x2000) >> 13),
        str((post_fault_lo & 0x4000) >> 14),
        str((post_fault_lo & 0x8000) >> 15),
        "0x" + raw_message[4:8],
        str(post_fault_hi & 0x0001),
        str((post_fault_hi & 0x0002) >> 1),
        str((post_fault_hi & 0x0004) >> 2),
        str((post_fault_hi & 0x0008) >> 3),
        str((post_fault_hi & 0x0010) >> 4),
        str((post_fault_hi & 0x0020) >> 5),
        str((post_fault_hi & 0x0040) >> 6),
        str((post_fault_hi & 0x0080) >> 7),
        str((post_fault_hi & 0x0100) >> 8),
        str((post_fault_hi & 0x0200) >> 9),
        str((post_fault_hi & 0x0400) >> 10),
        str((post_fault_hi & 0x0800) >> 11),
        str((post_fault_hi & 0x1000) >> 12),
        str((post_fault_hi & 0x2000) >> 13),
        str((post_fault_hi & 0x4000) >> 14),
        str((post_fault_hi & 0x8000) >> 15),
        "0x" + raw_message[8:12],
        str(run_fault_lo & 0x0001),
        str((run_fault_lo & 0x0002) >> 1),
        str((run_fault_lo & 0x0004) >> 2),
        str((run_fault_lo & 0x0008) >> 3),
        str((run_fault_lo & 0x0010) >> 4),
        str((run_fault_lo & 0x0020) >> 5),
        str((run_fault_lo & 0x0040) >> 6),
        str((run_fault_lo & 0x0080) >> 7),
        str((run_fault_lo & 0x0100) >> 8),
        str((run_fault_lo & 0x0200) >> 9),
        str((run_fault_lo & 0x0400) >> 10),
        str((run_fault_lo & 0x0800) >> 11),
        str((run_fault_lo & 0x1000) >> 12),
        str((run_fault_lo & 0x2000) >> 13),
        str((run_fault_lo & 0x4000) >> 14),
        str((run_fault_lo & 0x8000) >> 15),
        "0x" + raw_message[12:16],
        str(run_fault_hi & 0x0001),
        str((run_fault_hi & 0x0002) >> 1),
        str((run_fault_hi & 0x0004) >> 2),
        str((run_fault_hi & 0x0008) >> 3),
        str((run_fault_hi & 0x0010) >> 4),
        str((run_fault_hi & 0x0020) >> 5),
        str((run_fault_hi & 0x0040) >> 6),
        str((run_fault_hi & 0x0080) >> 7),
        str((run_fault_hi & 0x0100) >> 8),
        str((run_fault_hi & 0x0200) >> 9),
        str((run_fault_hi & 0x0400) >> 10),
        str((run_fault_hi & 0x0800) >> 11),
        str((run_fault_hi & 0x1000) >> 12),
        str((run_fault_hi & 0x2000) >> 13),
        str((run_fault_hi & 0x4000) >> 14),
        str((run_fault_hi & 0x8000) >> 15)
    ]

    units = []
    for i in range(len(labels)):
        units.append("")
  
    return [message, labels, values, units]

def parse_ID_MC_TORQUE_TIMER_INFORMATION(raw_message):
    message = "MC_torque_timer_information"
    labels = ["commanded_torque", "torque_feedback", "rms_uptime"]
    values = [hex_to_decimal(raw_message[0:4], 16, True) / 10.0, hex_to_decimal(raw_message[4:8], 16, True) / 10.0, hex_to_decimal(raw_message[8:16], 32, False)]
    units = ["C", "C", "s"]
    return [message, labels, values, units]

def parse_ID_MC_FLUX_WEAKENING_OUTPUT(raw_message):
    message = "MC_flux_weakening_output"
    labels = ["modulation_index", "flux_weakening_output", "id_command", "iq_command"]
    values = ["0x" + raw_message[2:4] + raw_message[0:2], "0x" + raw_message[6:8] + raw_message[4:6], hex_to_decimal(raw_message[8:12], 16, True), hex_to_decimal(raw_message[12:16], 16, True)]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_FIRMWARE_INFORMATION(raw_message):
    message = "MC_firmware_information"
    labels = ["eeprom_version_project_code", "software_version", "date_code_mmdd", "date_code_yyyy"]
    values = [hex_to_decimal(raw_message[0:4], 16, False), hex_to_decimal(raw_message[4:8], 16, False), hex_to_decimal(raw_message[8:12], 16, False), hex_to_decimal(raw_message[12:16], 16, False)]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_DIAGNOSTIC_DATA(raw_message):
    if DEBUG: print("ERROR: Do not know how to parse CAN ID 0xAF. This CAN ID does not exist in the HyTech CAN Library.")
    return ["MC_flux_information", ["UNPARSEABLE"], ["UNPARSEABLE"], ["UNPARSEABLE"]]

def parse_ID_MC_COMMAND_MESSAGE(raw_message):
    message = "MC_command_message"
    labels = ["requested_torque", "angular_velocity", "direction", "inverter_enable", "discharge_enable", "command_torque_limit"]
    values = [hex_to_decimal(raw_message[0:4], 16, True) / 10.0, hex_to_decimal(raw_message[4:8], 16, True), "0x" + raw_message[9], hex_to_decimal(raw_message[10], 4, False), hex_to_decimal(raw_message[11], 4, False), hex_to_decimal(raw_message[12:16], 16, True)]
    units = []
    for i in range(len(labels)):
        units.append("")
    return [message, labels, values, units]


def parse_ID_MC_READ_WRITE_PARAMETER_COMMAND(raw_message):
    message = "MC_read_write_parameter_command"
    labels = ["parameter_address", "rw_command", "reserved1", "data"]
    values = [hex(int(hex_to_decimal(raw_message[0:4], 16, False))), hex(int(raw_message[5])), hex_to_decimal(raw_message[6:8], 8, False), hex(int(hex_to_decimal(raw_message[8:16], 32, False)))]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_READ_WRITE_PARAMETER_RESPONSE(raw_message):
    message = "MC_read_write_parameter_response"
    labels = ["parameter_address", "write_success", "reserved1", "data"]
    values = [hex(int(hex_to_decimal(raw_message[0:4], 16, False))), hex(int(raw_message[5])), hex_to_decimal(raw_message[6:8], 8, False), hex(int(hex_to_decimal(raw_message[8:16], 32, False)))]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MCU_STATUS(raw_message):
    message = "MCU_status"
    labels = [
        "imd_ok_high",
        "shutdown_b_above_threshold",
        "bms_ok_high",
        "shutdown_c_above_threshold",
        "bspd_ok_high",
        "shutdown_d_above_threshold",
        "software_ok_high",
        "shutdown_e_above_threshold",
        "no_accel_implausability",
        "no_brake_implausability",
        "brake_pedal_active",
        "bspd_current_high",
        "bspd_brake_high",
        "no_accel_brake_implausability",
        "mcu_state",
        "inverter_powered",
        "energy_meter_present",
        "activate_buzzer",
        "software_is_ok",
        "launch_ctrl_active",
        "max_torque",
        "torque_mode",
        "distance_travelled"
    ]

    temp_bin_rep = bin(hex_to_decimal(raw_message[2:4], 8, False))[2:].zfill(8) + bin(hex_to_decimal(raw_message[4:6], 8, False))[2:].zfill(8) + bin(hex_to_decimal(raw_message[6:8], 8, False))[2:].zfill(8)
    
    # Change bin rep to small endian
    bin_rep = ""
    for i in range(len(temp_bin_rep)):
        if i % 8 == 0:
            bin_rep = bin_rep + temp_bin_rep[i+7] + temp_bin_rep[i+6] + temp_bin_rep[i+5] + temp_bin_rep[4] + temp_bin_rep[i+3] + temp_bin_rep[i+2] + temp_bin_rep[i+1] + temp_bin_rep[i]

    def binary_to_MCU_STATE(bin_rep):
        # Back to Big Endian
        bin_rep = bin_rep[2] + bin_rep[1] + bin_rep[0]
        
        if bin_rep == "000": return "STARTUP"
        if bin_rep == "001": return "TRACTIVE_SYSTEM_NOT_ACTIVE"
        if bin_rep == "010": return "TRACTIVE_SYSTEM_ACTIVE"
        if bin_rep == "011": return "ENABLING_INVERTER"
        if bin_rep == "100": return "WAITING_READY_TO_DRIVE_SOUND"
        if bin_rep == "101": return "READY_TO_DRIVE"
        # Should never get here
        if True: print("ERROR: Unrecognized MCU state: " + bin_rep)
        return "UNRECOGNIZED_STATE"

    values = []
    for i in range(24):
        if i == 8 or i == 9: # torque_mode 2-bits are never used
            continue
        elif i == 16: # convert binary to MCU state
            values.append(binary_to_MCU_STATE(bin_rep[16:19]))
        elif i == 17 or i == 18: # no need to do anything here because i == 16 takes care of them
            continue
        else:
            values.append(bin_rep[i])
    values.append(hex_to_decimal(raw_message[8:10], 8, False))
    values.append(hex_to_decimal(raw_message[10:12], 8, False))
    values.append(hex_to_decimal(raw_message[12:16], 16, False) / 100.0)

    units = []
    for i in range(len(labels) - 3):
        units.append("")
    units.append("Nm")
    units.append("")
    units.append("m")

    return [message, labels, values, units]

def parse_ID_MCU_PEDAL_READINGS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_MCU_ANALOG_READINGS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_ONBOARD_TEMPERATURES(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_ONBOARD_DETAILED_TEMPERATURES(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_VOLTAGES(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_DETAILED_VOLTAGES(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_TEMPERATURES(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_DETAILED_TEMPERATURES(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_STATUS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_FH_WATCHDOG_TEST(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_CCU_STATUS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_BALANCING_STATUS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_READ_WRITE_PARAMETER_COMMAND(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_PARAMETER_RESPONSE(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_BMS_COULOMB_COUNTS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_MCU_GPS_READINGS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_MCU_WHEEL_SPEED(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_DASHBOARD_STATUS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_SAB_READINGS_FRONT(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_SAB_READINGS_REAR(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_EM_STATUS(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_EM_MEASUREMENT(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_IMU_ACCELEROMETER(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]
def parse_ID_IMU_GYROSCOPE(raw_message):
    return ["N/A", ["N/A"], ["N/A"], [""]]


########################################################################
# Custom Parsing Functions End
########################################################################

def parse_message(raw_id, raw_message):
    '''
    @brief: Calls the corresponding custom parsing function depending on the CAN ID.
    @inputs: A string representing the raw CAN ID and an string of a hexadecimal raw message.
    @return: A four-element list [message, label[], value[], unit[]] if CAN ID is valid, otherwise system exit.
    '''
    if raw_id == "A0": return parse_ID_MC_TEMPERATURES1(raw_message)
    if raw_id == "A1": return parse_ID_MC_TEMPERATURES2(raw_message)
    if raw_id == "A2": return parse_ID_MC_TEMPERATURES3(raw_message)
    if raw_id == "A3": return parse_ID_MC_ANALOG_INPUTS_VOLTAGES(raw_message)
    if raw_id == "A4": return parse_ID_MC_DIGITAL_INPUTS_STATUS(raw_message)
    if raw_id == "A5": return parse_ID_MC_MOTOR_POSITION_INFORMATION(raw_message)
    if raw_id == "A6": return parse_ID_MC_CURRENT_INFORMATION(raw_message)
    if raw_id == "A7": return parse_ID_MC_VOLTAGE_INFORMATION(raw_message)
    if raw_id == "A8": return parse_ID_MC_FLUX_INFORMATION(raw_message)
    if raw_id == "A9": return parse_ID_MC_INTERNAL_VOLTAGES(raw_message)
    if raw_id == "AA": return parse_ID_MC_INTERNAL_STATES(raw_message)
    if raw_id == "AB": return parse_ID_MC_FAULT_CODES(raw_message)
    if raw_id == "AC": return parse_ID_MC_TORQUE_TIMER_INFORMATION(raw_message)
    if raw_id == "AD": return parse_ID_MC_FLUX_WEAKENING_OUTPUT(raw_message)
    if raw_id == "AE": return parse_ID_MC_FIRMWARE_INFORMATION(raw_message)
    if raw_id == "AF": return parse_ID_MC_DIAGNOSTIC_DATA(raw_message)
    
    if raw_id == "C0": return parse_ID_MC_COMMAND_MESSAGE(raw_message)
    if raw_id == "C1": return parse_ID_MC_READ_WRITE_PARAMETER_COMMAND(raw_message)
    if raw_id == "C2": return parse_ID_MC_READ_WRITE_PARAMETER_RESPONSE(raw_message)
    if raw_id == "C3": return parse_ID_MCU_STATUS(raw_message)
    if raw_id == "C4": return parse_ID_MCU_PEDAL_READINGS(raw_message)
    if raw_id == "CC": return parse_ID_MCU_ANALOG_READINGS(raw_message)

    if raw_id == "D5": return parse_ID_BMS_ONBOARD_TEMPERATURES(raw_message)
    if raw_id == "D6": return parse_ID_BMS_ONBOARD_DETAILED_TEMPERATURES(raw_message)
    if raw_id == "D7": return parse_ID_BMS_VOLTAGES(raw_message)
    if raw_id == "D8": return parse_ID_BMS_DETAILED_VOLTAGES(raw_message)
    if raw_id == "D9": return parse_ID_BMS_TEMPERATURES(raw_message)
    if raw_id == "DA": return parse_ID_BMS_DETAILED_TEMPERATURES(raw_message)
    if raw_id == "DB": return parse_ID_BMS_STATUS(raw_message)
    if raw_id == "DC": return parse_ID_FH_WATCHDOG_TEST(raw_message)
    if raw_id == "DD": return parse_ID_CCU_STATUS(raw_message)
    if raw_id == "DE": return parse_ID_BMS_BALANCING_STATUS(raw_message)

    if raw_id == "E0": return parse_ID_BMS_READ_WRITE_PARAMETER_COMMAND(raw_message)
    if raw_id == "E1": return parse_ID_BMS_PARAMETER_RESPONSE(raw_message)
    if raw_id == "E2": return parse_ID_BMS_COULOMB_COUNTS(raw_message)
    if raw_id == "E7": return parse_ID_MCU_GPS_READINGS(raw_message)
    if raw_id == "EA": return parse_ID_MCU_WHEEL_SPEED(raw_message)
    if raw_id == "EB": return parse_ID_DASHBOARD_STATUS(raw_message)
    if raw_id == "EC": return parse_ID_SAB_READINGS_FRONT(raw_message)
    if raw_id == "ED": return parse_ID_SAB_READINGS_REAR(raw_message)

    if raw_id == "100": return parse_ID_EM_STATUS(raw_message)
    if raw_id == "400": return parse_ID_EM_MEASUREMENT(raw_message)
    if raw_id == "470": return parse_ID_IMU_ACCELEROMETER(raw_message)
    if raw_id == "471": return parse_ID_IMU_GYROSCOPE(raw_message)

    # Should not come to here if CAN ID was valid
    if DEBUG: print("ERROR: Invalid CAN ID: 0x" + raw_message)
    return "INVALID_ID"

def parse_time(raw_time):
    '''
    @brief: Converts raw time into human-readable time.
    @input: The raw time given by the raw data CSV.
    @return: A string representing the human-readable time.
    @TODO: add millisecond support
    '''
    raw_time = int(raw_time) / 1000
    time = str(datetime.utcfromtimestamp(raw_time).strftime('%Y-%m-%d %H:%M:%S'))
    return time

def parse_file(filename):
    '''
    @brief: Reads raw data file and creates parsed data CSV.
            Loops through lines to write to parsed datafile.
            Calls the parse_message and parse_time functions as helpers.
    @input: The filename of the raw and parsed CSV.
    @return: N/A
    '''

    infile = open("Raw_Data/" + filename, "r")
    outfile = open("Parsed_Data/" + filename, "w")

    flag_first_line = True
    for line in infile.readlines():
        # On the first line, do not try to parse. Instead, set up the CSV headers.
        if flag_first_line:
            flag_first_line = False
            outfile.write("time,id,message,label,value,unit\n")

        # Otherwise attempt to parse the line.
        else:
            raw_time = line.split(",")[0]
            raw_id = line.split(",")[1]
            length = line.split(",")[2]
            raw_message = line.split(",")[3]

            # Do not parse if the length of the message is 0, otherwise bugs will occur later.
            if length == 0 or raw_message == "\n":
                continue
            
            # Call helper functions
            time = parse_time(raw_time)
            raw_message = raw_message[:(int(length) * 2)] # Strip trailing end of line/file characters that may cause bad parsing
            raw_message = raw_message.zfill(16) # Sometimes messages come truncated if 0s on the left. Append 0s so field-width is 16.
            table = parse_message(raw_id, raw_message)
            if table == "INVALID_ID":
                continue

            # Assertions that check for parser failure. Notifies user on where parser broke.
            assert len(table) == 4, "ERROR: Parser expected 4 arguments from parse_message at ID: 0x" + table[0] + ", got: " + str(len(table))
            assert len(table[1]) == len (table[2]) and len(table[1]) == len(table[3]), "ERROR: Label, Data, or Unit numbers mismatch for ID: 0x" + raw_id
            
            # Harvest parsed datafields and write to outfile.
            message = table[0].strip()
            for i in range(len(table[1])):
                label = table[1][i].strip()
                value = str(table[2][i]).strip()
                unit = table[3][i].strip()

                outfile.write(time + ",0x" + raw_id + "," + message + "," + label + "," + value + "," + unit + "\n")

    infile.close()
    outfile.close()
    return

def parse_folder():
    '''
    @brief: Locates Raw_Data directory or else throws errors. Created Parsed_Data directory if not created.
            Calls the parse_file() function on each raw CSV and alerts the user of parsing progress.
    @input: N/A
    @return: N/A
    '''

    # Stop attempting to parse if Raw_Data is not there.
    try:
        directory = os.fsencode("Raw_Data")
    except:
        print("ERROR: Raw_Data folder does not exist. Please move parser.py or create Raw_Data folder.")
        sys.exit(0)

    # Creates Parsed_Data folder if not there.
    if not os.path.exists("Parsed_Data"):
        os.makedirs("Parsed_Data")

    print("Currently parsing, please be patient...")

    # Loops through files and call parse_file on each raw CSV.
    for file in os.listdir(directory):
        filename = os.fsdecode(file)
        if filename.endswith(".CSV") or filename.endswith(".csv"):
            parse_file(filename)
            print("Successfully parsed: " + filename)
        else:
            continue

    return 

########################################################################
# Entry Point to Framework
########################################################################
parse_folder()
print("SUCCESS: Parsing Complete.")