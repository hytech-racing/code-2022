"""
@Author: Bo Han Zhu
@Date: 1/15/2022
@Description: HyTech custom python parser. Reads CSVs from Raw_Data, parses them, and writes to Parsed_Data. Uses multiplier.py for multipliers.
@TODO: Also output to MATLAB struct
@TODO: Dashboard_status is not correct. Need more data to validate bit ordering.

parse_folder --> parse_file --> parse_time
                            --> parse_message --> parse_ID_XXXXXXXXX
"""

# Imports
import os
import sys
from multipliers import Multipliers
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

def bin_to_bool(bin):
    """
    @brief: Helper function to convert a single-digit binary to the string "true" or "false".
    @input: 0, 1, "0", or "1"
    @return: true if input is 1 or "1", false if 0 or "0", UNRECOGNIZED_BIN if neither
    """
    try:
        bin = int(bin)
    except:
        if DEBUG: print("UNFATAL ERROR: Binary conversion to boolean failed, received " + str(bin))
        return "UNRECOGNIZED_BIN"

    if bin == 0:
        return "false"
    elif bin == 1:
        return "true"
    else:
        if DEBUG: print("UNFATAL ERROR: Binary conversion to boolean failed, received " + str(bin))
        return "UNRECOGNIZED_BIN"


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
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.MC_TEMPERATURES1_MODULE_A_TEMPERATURE.value, 
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.MC_TEMPERATURES1_MODULE_B_TEMPERATURE.value, 
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.MC_TEMPERATURES1_MODULE_C_TEMPERATURE.value, 
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.MC_TEMPERATURES1_GATE_DRIVER_BOARD_TEMPERATURE.value
    ]
    units = ["C", "C", "C", "C"]
    return [message, labels, values, units]

def parse_ID_MC_TEMPERATURES2(raw_message):
    message = "MC_temperatures_2"
    labels = ["control_board_temperature", "rtd_1_temperature", "rtd_2_temperature", "rtd_3_temperature"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.MC_TEMPERATURES2_CONTROL_BOARD_TEMPERATURES.value, 
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.MC_TEMPERATURES2_RTD_1_TEMPERATURES.value, 
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.MC_TEMPERATURES2_RTD_2_TEMPERATURES.value, 
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.MC_TEMPERATURES2_RTD_3_TEMPERATURES.value
    ]
    units = ["C", "C", "C", "C"]
    return [message, labels, values, units]

def parse_ID_MC_TEMPERATURES3(raw_message):
    message = "MC_temperatures_3"
    labels = ["rtd_4_temperatures", "rtd_5_temperature", "motor_temperature", "torque_shudder"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.MC_TEMPERATURES3_RTD_4_TEMPERATURES.value,
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.MC_TEMPERATURES3_RTD_5_TEMPERATURES.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.MC_TEMPERATURES3_MOTOR_TEMPERATURE.value, 
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.MC_TEMPERATURES3_TORQUE_SHUDDER.value
    ]
    units = ["C", "C", "C", "N-m"]
    return [message, labels, values, units]

def parse_ID_MC_ANALOG_INPUTS_VOLTAGES(raw_message):
    message = "MC_analog_input_voltages"
    labels = ["MC_analog_input_1", "MC_analog_input_2", "MC_analog_input_3", "MC_analog_input_4"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True),
        hex_to_decimal(raw_message[4:8], 16, True),
        hex_to_decimal(raw_message[8:12], 16, True),
        hex_to_decimal(raw_message[12:16], 16, True)
    ]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_DIGITAL_INPUTS_STATUS(raw_message):
    message = "MC_digital_input_status"
    labels = [
        "MC_digital_input_1", 
        "MC_digital_input_2", 
        "MC_digital_input_3", 
        "MC_digital_input_4", 
        "MC_digital_input_5", 
        "MC_digital_input_6", 
        "MC_digital_input_7", 
        "MC_digital_input_8"
    ]
    values = [
        raw_message[1], 
        raw_message[3], 
        raw_message[5], 
        raw_message[7], 
        raw_message[9], 
        raw_message[11], 
        raw_message[13], 
        raw_message[15]
    ]
    units = ["", "", "", "", "", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_MOTOR_POSITION_INFORMATION(raw_message):
    message = "MC_motor_position_information"
    labels = ["motor_angle", "motor_speed", "elec_output_freq", "delta_resolver_filtered"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.MC_MOTOR_POSITION_INFORMATION_MOTOR_ANGLE.value,
        hex_to_decimal(raw_message[4:8], 16, True), 
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.MC_MOTOR_POSITION_INFORMATION_ELEC_OUTPUT_FREQ.value, 
        hex_to_decimal(raw_message[12:16], 16, True)
    ]
    units = ["", "RPM", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_CURRENT_INFORMATION(raw_message):
    message = "MC_current_information"
    labels = ["phase_a_current", "phase_b_current", "phase_c_current", "dc_bus_current"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.MC_CURRENT_INFORMATION_PHASE_A_CURRENT.value, 
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.MC_CURRENT_INFORMATION_PHASE_B_CURRENT.value, 
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.MC_CURRENT_INFORMATION_PHASE_C_CURRENT.value, 
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.MC_CURRENT_INFORMATION_DC_BUS_CURRENT.value
    ]
    units = ["A", "A", "A", "A"]
    return [message, labels, values, units]

def parse_ID_MC_VOLTAGE_INFORMATION(raw_message):
    message = "MC_voltage_information"
    labels = ["dc_bus_voltage", "output_voltage", "phase_ab_voltage", "phase_bc_voltage"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.MC_VOLTAGE_INFORMATION_DC_BUS_VOLTAGE.value,
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.MC_VOLTAGE_INFORMATION_OUTPUT_VOLTAGE.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.MC_VOLTAGE_INFORMATION_PHASE_AB_VOLTAGE.value,
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.MC_VOLTAGE_INFORMATION_PHASE_BC_VOLTAGE.value
    ]
    units = ["V", "V", "V", "V"]
    return [message, labels, values, units]

def parse_ID_MC_FLUX_INFORMATION(raw_message):
    if DEBUG: print("UNFATAL ERROR: Do not know how to parse CAN ID 0xA8.")
    return "UNPARSEABLE"
    
def parse_ID_MC_INTERNAL_VOLTAGES(raw_message):
    if DEBUG: print("UNFATAL ERROR: Do not know how to parse CAN ID 0xA9.")
    return "UNPARSEABLE"

def parse_ID_MC_INTERNAL_STATES(raw_message):
    message = "MC_internal_states"
    labels = [
        "vsm_state",
        "inverter_state", 
        "relay_active_1", 
        "relay_active_2", 
        "relay_active_3", 
        "relay_active_4", 
        "relay_active_5", 
        "relay_active_6", 
        "inverter_run_mode", 
        "inverter_active_discharge_state", 
        "inverter_command_mode", 
        "inverter_enable_state", 
        "inverter_enable_lockout", 
        "direction_command"
    ]
    
    relay_state = hex_to_decimal(raw_message[6:8], 8, False)
    relay_state_1 = bin_to_bool(str(relay_state & 0x01))
    relay_state_2 = bin_to_bool(str((relay_state & 0x02) >> 1))
    relay_state_3 = bin_to_bool(str((relay_state & 0x04) >> 2))
    relay_state_4 = bin_to_bool(str((relay_state & 0x08) >> 3))
    relay_state_5 = bin_to_bool(str((relay_state & 0x10) >> 4))
    relay_state_6 = bin_to_bool(str((relay_state & 0x20) >> 5))
    inverter_run_mode_discharge_state = hex_to_decimal(raw_message[8:10], 8, False)
    inverter_run_mode = bin_to_bool(str(inverter_run_mode_discharge_state & 1))
    inverter_active_discharge_status = bin_to_bool(str(inverter_run_mode_discharge_state >> 5))
    inverter_enable = hex_to_decimal(raw_message[12:14], 8, False)
    inverter_enable_state = bin_to_bool(str(inverter_enable & 1))
    inverter_enable_lockout = bin_to_bool(str((inverter_enable & 0x80) >> 7))

    values = [
        hex(hex_to_decimal(raw_message[0:4], 16, False)),
        hex(int(raw_message[4:6], 16)), 
        relay_state_1, 
        relay_state_2,
        relay_state_3, 
        relay_state_4, 
        relay_state_5, 
        relay_state_6, 
        inverter_run_mode, 
        inverter_active_discharge_status, 
        hex(int(raw_message[10:12], 16)), 
        inverter_enable_state, 
        inverter_enable_lockout, 
        hex(hex_to_decimal(raw_message[14:16], 16, False))
    ]
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
        hex(hex_to_decimal(raw_message[0:4], 16, False)),
        bin_to_bool(str(post_fault_lo & 0x0001)),
        bin_to_bool(str((post_fault_lo & 0x0002) >> 1)),
        bin_to_bool(str((post_fault_lo & 0x0004) >> 2)),
        bin_to_bool(str((post_fault_lo & 0x0008) >> 3)),
        bin_to_bool(str((post_fault_lo & 0x0010) >> 4)),
        bin_to_bool(str((post_fault_lo & 0x0020) >> 5)),
        bin_to_bool(str((post_fault_lo & 0x0040) >> 6)),
        bin_to_bool(str((post_fault_lo & 0x0080) >> 7)),
        bin_to_bool(str((post_fault_lo & 0x0100) >> 8)),
        bin_to_bool(str((post_fault_lo & 0x0200) >> 9)),
        bin_to_bool(str((post_fault_lo & 0x0400) >> 10)),
        bin_to_bool(str((post_fault_lo & 0x0800) >> 11)),
        bin_to_bool(str((post_fault_lo & 0x1000) >> 12)),
        bin_to_bool(str((post_fault_lo & 0x2000) >> 13)),
        bin_to_bool(str((post_fault_lo & 0x4000) >> 14)),
        bin_to_bool(str((post_fault_lo & 0x8000) >> 15)),
        hex(hex_to_decimal(raw_message[4:8], 16, False)),
        bin_to_bool(str(post_fault_hi & 0x0001)),
        bin_to_bool(str((post_fault_hi & 0x0002) >> 1)),
        bin_to_bool(str((post_fault_hi & 0x0004) >> 2)),
        bin_to_bool(str((post_fault_hi & 0x0008) >> 3)),
        bin_to_bool(str((post_fault_hi & 0x0010) >> 4)),
        bin_to_bool(str((post_fault_hi & 0x0020) >> 5)),
        bin_to_bool(str((post_fault_hi & 0x0040) >> 6)),
        bin_to_bool(str((post_fault_hi & 0x0080) >> 7)),
        bin_to_bool(str((post_fault_hi & 0x0100) >> 8)),
        bin_to_bool(str((post_fault_hi & 0x0200) >> 9)),
        bin_to_bool(str((post_fault_hi & 0x0400) >> 10)),
        bin_to_bool(str((post_fault_hi & 0x0800) >> 11)),
        bin_to_bool(str((post_fault_hi & 0x1000) >> 12)),
        bin_to_bool(str((post_fault_hi & 0x2000) >> 13)),
        bin_to_bool(str((post_fault_hi & 0x4000) >> 14)),
        bin_to_bool(str((post_fault_hi & 0x8000) >> 15)),
        hex(hex_to_decimal(raw_message[8:12], 16, False)),
        str(run_fault_lo & 0x0001),
        bin_to_bool(str((run_fault_lo & 0x0002) >> 1)),
        bin_to_bool(str((run_fault_lo & 0x0004) >> 2)),
        bin_to_bool(str((run_fault_lo & 0x0008) >> 3)),
        bin_to_bool(str((run_fault_lo & 0x0010) >> 4)),
        bin_to_bool(str((run_fault_lo & 0x0020) >> 5)),
        bin_to_bool(str((run_fault_lo & 0x0040) >> 6)),
        bin_to_bool(str((run_fault_lo & 0x0080) >> 7)),
        bin_to_bool(str((run_fault_lo & 0x0100) >> 8)),
        bin_to_bool(str((run_fault_lo & 0x0200) >> 9)),
        bin_to_bool(str((run_fault_lo & 0x0400) >> 10)),
        bin_to_bool(str((run_fault_lo & 0x0800) >> 11)),
        bin_to_bool(str((run_fault_lo & 0x1000) >> 12)),
        bin_to_bool(str((run_fault_lo & 0x2000) >> 13)),
        bin_to_bool(str((run_fault_lo & 0x4000) >> 14)),
        bin_to_bool(str((run_fault_lo & 0x8000) >> 15)),
        hex(hex_to_decimal(raw_message[12:16], 16, False)),
        bin_to_bool(str(run_fault_hi & 0x0001)),
        bin_to_bool(str((run_fault_hi & 0x0002) >> 1)),
        bin_to_bool(str((run_fault_hi & 0x0004) >> 2)),
        bin_to_bool(str((run_fault_hi & 0x0008) >> 3)),
        bin_to_bool(str((run_fault_hi & 0x0010) >> 4)),
        bin_to_bool(str((run_fault_hi & 0x0020) >> 5)),
        bin_to_bool(str((run_fault_hi & 0x0040) >> 6)),
        bin_to_bool(str((run_fault_hi & 0x0080) >> 7)),
        bin_to_bool(str((run_fault_hi & 0x0100) >> 8)),
        bin_to_bool(str((run_fault_hi & 0x0200) >> 9)),
        bin_to_bool(str((run_fault_hi & 0x0400) >> 10)),
        bin_to_bool(str((run_fault_hi & 0x0800) >> 11)),
        bin_to_bool(str((run_fault_hi & 0x1000) >> 12)),
        bin_to_bool(str((run_fault_hi & 0x2000) >> 13)),
        bin_to_bool(str((run_fault_hi & 0x4000) >> 14)),
        bin_to_bool(str((run_fault_hi & 0x8000) >> 15))
    ]

    units = []
    for i in range(len(labels)):
        units.append("")
  
    return [message, labels, values, units]

def parse_ID_MC_TORQUE_TIMER_INFORMATION(raw_message):
    message = "MC_torque_timer_information"
    labels = ["commanded_torque", "torque_feedback", "rms_uptime"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.MC_TORQUE_TIMER_INFORMATION_COMMANDED_TORQUE.value,
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.MC_TORQUE_TIMER_INFORMATION_TORQUE_FEEDBACK.value, 
        hex_to_decimal(raw_message[8:16], 32, False)
    ]
    units = ["C", "C", "ms"]
    return [message, labels, values, units]

def parse_ID_MC_FLUX_WEAKENING_OUTPUT(raw_message):
    message = "MC_flux_weakening_output"
    labels = ["modulation_index", "flux_weakening_output", "id_command", "iq_command"]
    values = [
        hex(hex_to_decimal(raw_message[0:4], 16, False)),
        hex(hex_to_decimal(raw_message[4:8], 16, False)),
        hex_to_decimal(raw_message[8:12], 16, True), 
        hex_to_decimal(raw_message[12:16], 16, True)
    ]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_FIRMWARE_INFORMATION(raw_message):
    message = "MC_firmware_information"
    labels = ["eeprom_version_project_code", "software_version", "date_code_mmdd", "date_code_yyyy"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, False),
        hex_to_decimal(raw_message[4:8], 16, False),
        hex_to_decimal(raw_message[8:12], 16, False),
        hex_to_decimal(raw_message[12:16], 16, False)
    ]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_DIAGNOSTIC_DATA(raw_message):
    if DEBUG: print("UNFATAL ERROR: Do not know how to parse CAN ID 0xAF.")
    return "UNPARSEABLE"

def parse_ID_MC_COMMAND_MESSAGE(raw_message):
    message = "MC_command_message"
    labels = ["requested_torque", "angular_velocity", "direction", "inverter_enable", "discharge_enable", "command_torque_limit"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.MC_COMMAND_MESSAGE_REQUESTED_TORQUE.value,
        hex_to_decimal(raw_message[4:8], 16, True), 
        hex(int(raw_message[9], 16)),
        hex_to_decimal(raw_message[10], 4, False), 
        hex_to_decimal(raw_message[11], 4, False), 
        hex_to_decimal(raw_message[12:16], 16, True)
    ]
    units = []
    for i in range(len(labels)):
        units.append("")
    return [message, labels, values, units]

def parse_ID_MC_READ_WRITE_PARAMETER_COMMAND(raw_message):
    message = "MC_read_write_parameter_command"
    labels = ["parameter_address", "rw_command", "reserved1", "data"]
    values = [
        hex(int(hex_to_decimal(raw_message[0:4], 16, False))),
        hex(int(raw_message[5])),
        hex_to_decimal(raw_message[6:8], 8, False),
        hex(int(hex_to_decimal(raw_message[8:16], 32, False)))
    ]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MC_READ_WRITE_PARAMETER_RESPONSE(raw_message):
    message = "MC_read_write_parameter_response"
    labels = ["parameter_address", "write_success", "reserved1", "data"]
    values = [
        hex(int(hex_to_decimal(raw_message[0:4], 16, False))),
        hex(int(raw_message[5])),
        hex_to_decimal(raw_message[6:8], 8, False),
        hex(int(hex_to_decimal(raw_message[8:16], 32, False)))
    ]
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
        if DEBUG: print("UNFATAL ERROR: Unrecognized MCU state: " + bin_rep)
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
            values.append(bin_to_bool(bin_rep[i]))
    values.append(hex_to_decimal(raw_message[8:10], 8, False))
    values.append(hex_to_decimal(raw_message[10:12], 8, False))
    values.append(hex_to_decimal(raw_message[12:16], 16, False) / Multipliers.MCU_STATUS_DISTANCE_TRAVELLED.value)

    units = []
    for i in range(len(labels) - 3):
        units.append("")
    units.append("Nm")
    units.append("")
    units.append("m")

    return [message, labels, values, units]

def parse_ID_MCU_PEDAL_READINGS(raw_message):
    message = "MCU_pedal_readings"
    labels = ["accelerator_pedal_1", "accelerator_pedal_2", "brake_transducer_1", "brake_transducer_2"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, False), 
        hex_to_decimal(raw_message[4:8], 16, False), 
        hex_to_decimal(raw_message[8:12], 16, False), 
        hex_to_decimal(raw_message[12:16], 16, False)
    ]
    units = ["", "", "", ""]
    return [message, labels, values, units]

def parse_ID_MCU_ANALOG_READINGS(raw_message):
    message = "MCU_analog_readings"
    labels = ["ecu_current", "cooling_current", "temperature", "glv_battery_voltage"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, False) / Multipliers.MCU_ANALOG_READINGS_ECU_CURRENT.value, 
        hex_to_decimal(raw_message[4:8], 16, False) / Multipliers.MCU_ANALOG_READINGS_COOLING_CURRENT.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.MCU_ANALOG_READINGS_TEMPERATURE.value, 
        hex_to_decimal(raw_message[12:16], 16, False) / Multipliers.MCU_ANALOG_READINGS_GLV_BATTERY_VOLTAGE.value
    ]
    units = ["A", "A", "C", "V"]
    return [message, labels, values, units]

def parse_ID_BMS_ONBOARD_TEMPERATURES(raw_message):
    message = "BMS_onboard_temperatures"
    labels = ["average_temperature", "low_temperature", "high_temperature"]
    values = [
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.BMS_ONBOARD_TEMPERATURES_AVERAGE_TEMPERATURE.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.BMS_ONBOARD_TEMPERATURES_LOW_TEMPERATURE.value,
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.BMS_ONBOARD_TEMPERATURES_HIGH_TEMPERATURE.value
    ]
    units = ["C", "C", "C"]
    return [message, labels, values, units]

def parse_ID_BMS_ONBOARD_DETAILED_TEMPERATURES(raw_message):
    message = "BMS_onboard_detailed_temperatures"
    ic_id = str(hex_to_decimal(raw_message[0:2], 8, False))
    labels = ["IC_" + ic_id + "_temperature_0", "IC_" + ic_id + "_temperature_1"]
    values = [
        hex_to_decimal(raw_message[2:6], 16, True) / Multipliers.BMS_ONBOARD_DETAILED_TEMPERATURES_TEMPERATURE_0.value,
        hex_to_decimal(raw_message[6:10], 16, True) / Multipliers.BMS_ONBOARD_DETAILED_TEMPERATURES_TEMPERATURE_1.value
    ]
    units = ["C", "C"]
    return [message, labels, values, units]

def parse_ID_BMS_VOLTAGES(raw_message):
    message = "BMS_voltages"
    labels = ["BMS_voltage_average", "BMS_voltage_low", "BMS_voltage_high", "BMS_voltage_total"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, False) / Multipliers.BMS_VOLTAGES_BMS_VOLTAGE_AVERAGE.value,
        hex_to_decimal(raw_message[4:8], 16, False) / Multipliers.BMS_VOLTAGES_BMS_VOLTAGE_LOW.value,
        hex_to_decimal(raw_message[8:12], 16, False) / Multipliers.BMS_VOLTAGES_BMS_VOLTAGE_HIGH.value,
        hex_to_decimal(raw_message[12:16], 16, False) / Multipliers.BMS_VOLTAGES_BMS_VOLTAGE_TOTAL.value
    ]
    units = ["V", "V", "V", "V"]
    return [message, labels, values, units]

def parse_ID_BMS_DETAILED_VOLTAGES(raw_message):
    message = "BMS_detailed_voltages"
    ic_id = raw_message[1]
    group_id = int(raw_message[0])
    labels = ""
    if group_id == 0:
        labels = ["IC_" + ic_id + "_CELL_0", "IC_" + ic_id + "_CELL_1", "IC_" + ic_id + "_CELL_2"]
    elif group_id == 1:
        labels = ["IC_" + ic_id + "_CELL_3", "IC_" + ic_id + "_CELL_4", "IC_" + ic_id + "_CELL_5"]
    elif group_id == 2:
        labels = ["IC_" + ic_id + "_CELL_6", "IC_" + ic_id + "_CELL_7", "IC_" + ic_id + "_CELL_8"]
    else:
        if DEBUG: print("UNFATAL ERROR: BMS detailed voltage group " + str(group_id) + " is invalid.")
        return "UNPARSEABLE"
    values = [
        hex_to_decimal(raw_message[2:6], 16, False) / Multipliers.BMS_DETAILED_VOLTAGES_VOLTAGE_0.value,
        hex_to_decimal(raw_message[6:10], 16, False) / Multipliers.BMS_DETAILED_VOLTAGES_VOLTAGE_1.value,
        hex_to_decimal(raw_message[10:14], 16, False) / Multipliers.BMS_DETAILED_VOLTAGES_VOLTAGE_2.value
    ]
    units = ["V", "V", "V"]
    return [message, labels, values, units]

def parse_ID_BMS_TEMPERATURES(raw_message):
    message = "BMS_temperatures"
    labels = ["BMS_average_temperature", "BMS_low_temperature", "BMS_high_temperature"]
    values = [
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.BMS_TEMPERATURES_BMS_AVERAGE_TEMPERATURE.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.BMS_TEMPERATURES_BMS_LOW_TEMPERATURE.value,
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.BMS_TEMPERATURES_BMS_HIGH_TEMPERATURE.value
    ]
    units = ["C", "C", "C"]
    return [message, labels, values, units]

def parse_ID_BMS_DETAILED_TEMPERATURES(raw_message):
    message = "BMS_detailed_temperatures"
    ic_id = str(hex_to_decimal(raw_message[0:2], 8, False))
    labels = ["IC_" + ic_id + "_therm_0", "IC_" + ic_id + "_therm_1", "IC_" + ic_id + "_therm_2"]
    values = [
        hex_to_decimal(raw_message[2:6], 16, True) / Multipliers.BMS_DETAILED_TEMPERATURES_THERM_0.value,
        hex_to_decimal(raw_message[6:10], 16, True) / Multipliers.BMS_DETAILED_TEMPERATURES_THERM_1.value,
        hex_to_decimal(raw_message[10:14], 16, True) / Multipliers.BMS_DETAILED_TEMPERATURES_THERM_2.value
    ]
    units = ["C", "C", "C"]
    return [message, labels, values, units]
    
def parse_ID_BMS_STATUS(raw_message):
    message = "BMS_status"
    labels = [
        "BMS_state",
        "BMS_error_flags",
        "BMS_overvoltage",
        "BMS_undervoltage",
        "BMS_total_voltage_high",
        "BMS_discharge_overcurrent",
        "BMS_charge_overcurrent",
        "BMS_discharge_overtemp",
        "BMS_charge_overtemp",
        "BMS_undertemp",
        "BMS_onboard_overtemp",
        "BMS_current",
        "BMS_flags",
        "BMS_shutdown_g_above_threshold",
        "BMS_shutdown_h_above_threshold"
    ]

    error_flags = hex_to_decimal(raw_message[6:10], 16, False)
    flags = hex_to_decimal(raw_message[14:16], 8, False)
    values = [
        hex(int(raw_message[4:6], 16)),
        hex(error_flags),
        bin_to_bool(error_flags & 0x1),
        bin_to_bool((error_flags & 0x2) >> 1),
        bin_to_bool((error_flags & 0x4) >> 2),
        bin_to_bool((error_flags & 0x8) >> 3),
        bin_to_bool((error_flags & 0x10) >> 4),
        bin_to_bool((error_flags & 0x20) >> 5),
        bin_to_bool((error_flags & 0x40) >> 6),
        bin_to_bool((error_flags & 0x80) >> 7),
        bin_to_bool((error_flags & 0x100) >> 8),
        hex_to_decimal(raw_message[10:14], 16, True) / Multipliers.BMS_STATUS_BMS_CURRENT.value,
        hex(int(raw_message[14:16], 16)),
        bin_to_bool(flags & 0x1),
        bin_to_bool((flags & 0x2) >> 1)
    ]

    units = []
    for i in range(len(labels)):
        if i == len(labels) - 4:
            units.append("A")
        else:
            units.append("")

    return [message, labels, values, units]

def parse_ID_FH_WATCHDOG_TEST(raw_message):
    if DEBUG: print("UNFATAL ERROR: Do not know how to parse CAN ID 0xDC.")
    return "UNPARSEABLE"

def parse_ID_CCU_STATUS(raw_message):
    message = "CCU_status"
    labels = ["charger_enabled"]
    values = [int(raw_message[14:16], 16)]
    units = [""]
    return [message, labels, values, units]

def parse_ID_BMS_BALANCING_STATUS(raw_message):
    if DEBUG: print("UNFATAL ERROR: Do not know how to parse CAN ID 0xDE.")
    return "UNPARSEABLE"

def parse_ID_BMS_READ_WRITE_PARAMETER_COMMAND(raw_message):
    if DEBUG: print("UNFATAL ERROR: Do not know how to parse CAN ID 0xE0.")
    return "UNPARSEABLE"

def parse_ID_BMS_PARAMETER_RESPONSE(raw_message):
    if DEBUG: print("UNFATAL ERROR: Do not know how to parse CAN ID 0xE1.")
    return "UNPARSEABLE"

def parse_ID_BMS_COULOMB_COUNTS(raw_message):
    message = "BMS_coulomb_counts"
    labels = ["BMS_total_charge", "BMS_total_discharge"]
    values = [
        hex_to_decimal(raw_message[0:8], 32, False) / Multipliers.BMS_COULOMB_COUNTS_BMS_TOTAL_CHARGE.value,
        hex_to_decimal(raw_message[8:16], 32, False) / Multipliers.BMS_COULOMB_COUNTS_BMS_TOTAL_DISCHARGE.value
    ]
    units = ["C", "C"]
    return [message, labels, values, units]

def parse_ID_MCU_GPS_READINGS(raw_message):
    message = "MCU_GPS_readings"
    labels = ["latitude", "longitude"]
    values = [
        hex_to_decimal(raw_message[0:8], 32, True) / Multipliers.MCU_GPS_READINGS_LATITUDE.value,
        hex_to_decimal(raw_message[8:16], 32, True) / Multipliers.MCU_GPS_READINGS_LONGITUDE.value
    ]
    units = ["deg", "deg"]
    return [message, labels, values, units]

def parse_ID_MCU_WHEEL_SPEED(raw_message):
    message = "MCU_wheel_speed"
    labels = ["rpm_front_left", "rpm_front_right", "rpm_back_left", "rpm_back_right"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, False) / Multipliers.MCU_WHEEL_SPEED_RPM_FRONT_LEFT.value,
        hex_to_decimal(raw_message[4:8], 16, False) / Multipliers.MCU_WHEEL_SPEED_RPM_FRONT_RIGHT.value,
        hex_to_decimal(raw_message[8:12], 16, False) / Multipliers.MCU_WHEEL_SPEED_RPM_BACK_LEFT.value,
        hex_to_decimal(raw_message[12:16], 16, False) / Multipliers.MCU_WHEEL_SPEED_RPM_BACK_RIGHT.value
    ]
    units = ["rpm", "rpm", "rpm", "rpm"]
    return [message, labels, values, units]

# @TODO: FIX THIS with more data
def parse_ID_DASHBOARD_STATUS(raw_message):
    message = "Dashboard_status"
    labels = [
        "start_btn",
        "buzzer_active",
        "ssok_above_threshold",
        "shutdown_h_above_threshold",
        "mark_btn",
        "mode_btn",
        "mc_cycle_btn",
        "launch_ctrl_btn",
        "ams_led",
        "imd_led",
        "mode_led",
        "mc_error_led",
        "start_led",
        "launch_control_led"
    ]
    
    raw_message = raw_message[8:] # Strip first 8 padded zeros
    bin_rep = bin(int(raw_message, 16))[2:].zfill(32)
    def blink_modes(bin_rep):
        bin_rep = int(bin(bin_rep)[2:].zfill(2)[1] + bin(bin_rep)[2:].zfill(2)[0], 2) # Back to big-endian
        if bin_rep == 0: return "off"
        elif bin_rep == 1: return "on"
        elif bin_rep == 2: return "fast"
        elif bin_rep == 3: return "slow"
        else:
            if DEBUG: print("UNFATAL ERROR: Unrecognizable blink mode " + str(bin_rep))
            return "UNRECOGNIZED_BLINK"

    led_flags = (bin_rep[30:32] + bin_rep[28:30] + bin_rep[26:28] + bin_rep[24:26])[::-1] + (bin_rep[22:24] + bin_rep[20:22] + bin_rep[18:20] + bin_rep[16:18])[::-1]
    values = [
        bin_to_bool(bin_rep[7]), # Endianness changed
        bin_to_bool(bin_rep[6]),
        bin_to_bool(bin_rep[5]),
        bin_to_bool(bin_rep[4]),
        bin_to_bool(bin_rep[15]),
        bin_to_bool(bin_rep[14]),
        bin_to_bool(bin_rep[13]),
        bin_to_bool(bin_rep[12]),
        blink_modes(int(led_flags, 2) & 0x0003),
        blink_modes((int(led_flags, 2) & 0x000C) >> 2),
        blink_modes((int(led_flags, 2) & 0x0030) >> 4),
        blink_modes((int(led_flags, 2) & 0x00C0) >> 6),
        blink_modes((int(led_flags, 2) & 0x0300) >> 8),
        blink_modes((int(led_flags, 2) & 0x0C00) >> 10)
    ]

    units = []
    for i in range(len(labels)):
        units.append("")
    return [message, labels, values, units]

def parse_ID_SAB_READINGS_FRONT(raw_message):
    message = "SAB_readings_front"
    labels = ["fl_susp_lin_pot", "fr_susp_lin_pot", "steer_wheel_sensor", "amb_air_hum"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.SAB_READINGS_ALL.value,
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.SAB_READINGS_ALL.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.SAB_READINGS_ALL.value,
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.SAB_READINGS_ALL.value
    ]
    units = ["mm", "mm", "", "%"]
    return [message, labels, values, units]

def parse_ID_SAB_READINGS_REAR(raw_message):
    message = "SAB_readings_rear"
    labels = ["bl_susp_lin_pot", "br_susp_lin_pot", "amb_air_temp", "mc_cool_fluid_temp"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.SAB_READINGS_ALL.value,
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.SAB_READINGS_ALL.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.SAB_READINGS_ALL.value,
        hex_to_decimal(raw_message[12:16], 16, True) / Multipliers.SAB_READINGS_ALL.value
    ]
    units = ["mm", "mm", "C", "C"]
    return [message, labels, values, units]

def parse_ID_EM_MEASUREMENT(raw_message):
    message = "EM_measurement"
    labels = ["current", "voltage"]

    def twos_comp(value):
        bits = 32
        if value & (1 << (bits - 1)):
            value -= 1 << bits
        return value
    bin_rep = bin(int(raw_message, 16))
    bin_rep = bin_rep[2:].zfill(64)
    voltage = twos_comp(int(bin_rep[7:39], 2)) / Multipliers.EM_MEASUREMENTS_VOLTAGE.value
    current = twos_comp(int(bin_rep[39:71], 2)) / Multipliers.EM_MEASUREMENTS_CURRENT.value
    values = [voltage, current]

    units = ["A", "V"]
    return [message, labels, values, units]

def parse_ID_EM_STATUS(raw_message):
    message = "EM_status"
    labels = ["voltage_gain", "current_gain", "overvoltage", "overpower", "logging"]

    raw_message = raw_message[8:]
    bin_rep = bin(int(raw_message, 16))
    bin_rep = bin_rep[2:].zfill(32)
    voltage_gain = int(bin_rep[0:4], 2)
    current_gain = int(bin_rep[4:8], 2)
    if voltage_gain == 0:
        voltage_gain = "x1"
    elif voltage_gain == 1:
        voltage_gain = "x2"
    elif voltage_gain == 2:
        voltage_gain = "x4"
    elif voltage_gain == 3:
        voltage_gain = "x8"
    elif voltage_gain == 4:
        voltage_gain = "x16"
    elif voltage_gain == 5:
        voltage_gain = "x32"
    else:
        if DEBUG: print("UNFATAL ERROR: Unknown Energy Meter voltage gain: " + str(voltage_gain))
        voltage_gain = "N/A"
    
    if current_gain == 0:
        current_gain = "x1"
    elif current_gain == 1:
        current_gain = "x2"
    elif current_gain == 2:
        current_gain = "x4"
    elif current_gain == 3:
        current_gain = "x8"
    elif current_gain == 4:
        current_gain = "x16"
    elif current_gain == 5:
        current_gain = "x32"
    else:
        if DEBUG: print("UNFATAL ERROR: Unknown Energy Meter current gain: " + str(current_gain))
        current_gain = "N/A"

    values = [
        voltage_gain,
        current_gain,
        bin_to_bool(bin_rep[8]),
        bin_to_bool(bin_rep[9]),
        bin_to_bool(bin_rep[10])
    ]

    units = ["", "", "", "", ""]
    return [message, labels, values, units]

def parse_ID_IMU_ACCELEROMETER(raw_message):
    message = "IMU_accelerometer"
    labels = ["lat_accel", "long_accel", "vert_accel"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.IMU_ACCELEROMETER_ALL.value,
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.IMU_ACCELEROMETER_ALL.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.IMU_ACCELEROMETER_ALL.value
    ]
    units = ["m/s/s", "m/s/s", "m/s/s"]
    return [message, labels, values, units]

def parse_ID_IMU_GYROSCOPE(raw_message):
    message = "IMU_gyroscope"
    labels = ["yaw", "pitch", "roll"]
    values = [
        hex_to_decimal(raw_message[0:4], 16, True) / Multipliers.IMU_GYROSCOPE_ALL.value,
        hex_to_decimal(raw_message[4:8], 16, True) / Multipliers.IMU_GYROSCOPE_ALL.value,
        hex_to_decimal(raw_message[8:12], 16, True) / Multipliers.IMU_GYROSCOPE_ALL.value
    ]
    units = ["deg/s", "deg/s", "deg/s"]
    return [message, labels, values, units]


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

    if raw_id == "100": return parse_ID_EM_MEASUREMENT(raw_message)
    if raw_id == "400": return parse_ID_EM_STATUS(raw_message)
    if raw_id == "470": return parse_ID_IMU_ACCELEROMETER(raw_message)
    if raw_id == "471": return parse_ID_IMU_GYROSCOPE(raw_message)

    # Should not come to here if CAN ID was valid
    if DEBUG: print("UNFATAL ERROR: Invalid CAN ID: 0x" + raw_message)
    return "INVALID_ID"

def parse_time(raw_time):
    '''
    @brief: Converts raw time into human-readable time.
    @input: The raw time given by the raw data CSV.
    @return: A string representing the human-readable time.
    '''
    ms = int(raw_time) % 1000
    raw_time = int(raw_time) / 1000
    time = str(datetime.utcfromtimestamp(raw_time).strftime('%Y-%m-%dT%H:%M:%S'))
    time = time + "." + str(ms).zfill(3) + "Z"
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
            if table == "INVALID_ID" or table == "UNPARSEABLE":
                continue

            # Assertions that check for parser failure. Notifies user on where parser broke.
            assert len(table) == 4, "FATAL ERROR: Parser expected 4 arguments from parse_message at ID: 0x" + table[0] + ", got: " + str(len(table))
            assert len(table[1]) == len (table[2]) and len(table[1]) == len(table[3]), "FATAL ERROR: Label, Data, or Unit numbers mismatch for ID: 0x" + raw_id
            
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
        print("FATAL ERROR: Raw_Data folder does not exist. Please move parser.py or create Raw_Data folder.")
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