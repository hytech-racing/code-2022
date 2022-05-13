"""
@Author: Bo Han Zhu
@Date: 1/16/2022
@Description: Custom multipliers for different labels. If a multiplier does not exist for a label, it is just 1.
              Make all constants floats for best usage.
"""
from enum import Enum

class Multipliers(Enum):
    MC_TEMPERATURES1_MODULE_A_TEMPERATURE = 10.0
    MC_TEMPERATURES1_MODULE_B_TEMPERATURE = 10.0
    MC_TEMPERATURES1_MODULE_C_TEMPERATURE = 10.0
    MC_TEMPERATURES1_GATE_DRIVER_BOARD_TEMPERATURE = 10.0

    MC_TEMPERATURES2_CONTROL_BOARD_TEMPERATURES = 10.0
    MC_TEMPERATURES2_RTD_1_TEMPERATURES = 10.0
    MC_TEMPERATURES2_RTD_2_TEMPERATURES = 10.0
    MC_TEMPERATURES2_RTD_3_TEMPERATURES = 10.0

    MC_TEMPERATURES3_RTD_4_TEMPERATURES = 10.0
    MC_TEMPERATURES3_RTD_5_TEMPERATURES = 10.0
    MC_TEMPERATURES3_MOTOR_TEMPERATURE = 10.0
    MC_TEMPERATURES3_TORQUE_SHUDDER = 10.0

    MC_MOTOR_POSITION_INFORMATION_MOTOR_ANGLE = 10.0
    MC_MOTOR_POSITION_INFORMATION_ELEC_OUTPUT_FREQ = 10.0

    MC_CURRENT_INFORMATION_PHASE_A_CURRENT = 10.0
    MC_CURRENT_INFORMATION_PHASE_B_CURRENT = 10.0
    MC_CURRENT_INFORMATION_PHASE_C_CURRENT = 10.0
    MC_CURRENT_INFORMATION_DC_BUS_CURRENT = 10.0

    MC_VOLTAGE_INFORMATION_DC_BUS_VOLTAGE = 10.0
    MC_VOLTAGE_INFORMATION_OUTPUT_VOLTAGE = 10.0
    MC_VOLTAGE_INFORMATION_PHASE_AB_VOLTAGE = 10.0
    MC_VOLTAGE_INFORMATION_PHASE_BC_VOLTAGE = 10.0 

    MC_TORQUE_TIMER_INFORMATION_COMMANDED_TORQUE = 10.0
    MC_TORQUE_TIMER_INFORMATION_TORQUE_FEEDBACK = 10.0
    MC_TORQUE_TIMER_INFORMATION_RMS_UPTIME = 1000.0

    MC_FLUX_WEAKENING_OUTPUT_IQ_COMMAND = 10.0
    MC_FLUX_WEAKENING_OUTPUT_ID_COMMAND = 10.0
    
    MC_COMMAND_MESSAGE_REQUESTED_TORQUE = 10.0

    MCU_STATUS_DISTANCE_TRAVELLED = 100.0

    MCU_PEDAL_READINGS_ACCELERATOR_PEDAL_1 = 1.0 / (5.0 / 4096.0 * 4000.0/33.0)   # 5 (5V ref) / 4096 (12-bit ADC), others from linear equation
    MCU_PEDAL_READINGS_ACCELERATOR_PEDAL_2 = 1.0 / (5.0 / 4096.0 * 2500.0/11.0)   # 5 (5V ref) / 4096 (12-bit ADC), others from linear equation
    MCU_PEDAL_READINGS_BRAKE_TRANDUCER_1 = 1.0 / (5.0 / 4096.0 * 15000.0 / 5.0) # 5 (5V ref) / 4096 (12-bit ADC) * 15000 (psi) / 5 (supply V)
    MCU_PEDAL_READINGS_BRAKE_TRANDUCER_2 = 1.0 / (5.0 / 4096.0 * 15000.0 / 5.0) # 5 (5V ref) / 4096 (12-bit ADC) * 15000 (psi) / 5 (supply V)

    MCU_ANALOG_READINGS_ECU_CURRENT = 5000.0
    MCU_ANALOG_READINGS_COOLING_CURRENT = 5000.0
    MCU_ANALOG_READINGS_TEMPERATURE = 100.0
    MCU_ANALOG_READINGS_GLV_BATTERY_VOLTAGE = 2500.0

    BMS_ONBOARD_TEMPERATURES_AVERAGE_TEMPERATURE = 100.0
    BMS_ONBOARD_TEMPERATURES_LOW_TEMPERATURE = 100.0
    BMS_ONBOARD_TEMPERATURES_HIGH_TEMPERATURE = 100.0

    BMS_ONBOARD_DETAILED_TEMPERATURES_TEMPERATURE_0 = 100.0
    BMS_ONBOARD_DETAILED_TEMPERATURES_TEMPERATURE_1 = 100.0

    BMS_VOLTAGES_BMS_VOLTAGE_AVERAGE = 10000.0
    BMS_VOLTAGES_BMS_VOLTAGE_LOW = 10000.0
    BMS_VOLTAGES_BMS_VOLTAGE_HIGH = 10000.0
    BMS_VOLTAGES_BMS_VOLTAGE_TOTAL = 100.0

    BMS_DETAILED_VOLTAGES_VOLTAGE_0 = 10000.0
    BMS_DETAILED_VOLTAGES_VOLTAGE_1 = 10000.0
    BMS_DETAILED_VOLTAGES_VOLTAGE_2 = 10000.0

    BMS_TEMPERATURES_BMS_AVERAGE_TEMPERATURE = 100.0
    BMS_TEMPERATURES_BMS_LOW_TEMPERATURE = 100.0
    BMS_TEMPERATURES_BMS_HIGH_TEMPERATURE = 100.0
    
    BMS_DETAILED_TEMPERATURES_THERM_0 = 100.0
    BMS_DETAILED_TEMPERATURES_THERM_1 = 100.0
    BMS_DETAILED_TEMPERATURES_THERM_2 = 100.0

    BMS_STATUS_BMS_CURRENT = 100.0

    BMS_COULOMB_COUNTS_BMS_TOTAL_CHARGE = 10000.0
    BMS_COULOMB_COUNTS_BMS_TOTAL_DISCHARGE = 10000.0

    MCU_GPS_READINGS_LATITUDE = 100000.0
    MCU_GPS_READINGS_LONGITUDE = 100000.0

    MCU_WHEEL_SPEED_RPM_FRONT_LEFT = 10.0
    MCU_WHEEL_SPEED_RPM_FRONT_RIGHT = 10.0
    MCU_WHEEL_SPEED_RPM_BACK_LEFT = 10.0
    MCU_WHEEL_SPEED_RPM_BACK_RIGHT = 10.0

    SAB_READINGS_NON_GPS = 1000.0
    SAB_READINGS_GPS = 1000000.0
    
    EM_MEASUREMENTS_VOLTAGE = 65536.0
    EM_MEASUREMENTS_CURRENT = 65536.0

    IMU_ACCELEROMETER_ALL = 1000.0 / 9.813
    IMU_GYROSCOPE_ALL = 1000.0

