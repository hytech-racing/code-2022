#ifndef __HYTECH_CAN_H__
#define __HYTECH_CAN_H__

// ECU state definitions // TODO make these enums?
#define BMS_STATE_DISCHARGING 1
#define BMS_STATE_CHARGING 2
#define BMS_STATE_BALANCING 3
#define BMS_STATE_BALANCING_OVERHEATED 4

#include "CAN_ID.h"

#include "BMS_balancing_status.h"
#include "BMS_coulomb_counts.h"
#include "BMS_detailed_temperatures.h"
#include "BMS_detailed_voltages.h"
#include "BMS_onboard_detailed_temperatures.h"
#include "BMS_onboard_temperatures.h"
#include "BMS_status.h"
#include "BMS_temperatures.h"
#include "BMS_voltages.h"
#include "CCU_status.h"
#include "Dashboard_status.h"
#include "MCU_GPS_readings.h"
#include "MCU_pedal_readings.h"
#include "MCU_status.h"
#include "MCU_wheel_speed.h"
#include "MCU_analog_readings.h"
#include "MC_analog_input_voltages.h"
#include "MC_command_message.h"
#include "MC_current_information.h"
#include "MC_digital_input_status.h"
#include "MC_fault_codes.h"
#include "MC_flux_weakening_output.h"
#include "MC_firmware_information.h"
#include "MC_internal_states.h"
#include "MC_motor_position_information.h"
#include "MC_read_write_parameter_command.h"
#include "MC_read_write_parameter_response.h"
#include "MC_temperatures_1.h"
#include "MC_temperatures_2.h"
#include "MC_temperatures_3.h"
#include "MC_torque_timer_information.h"
#include "MC_voltage_information.h"
#include "SAB_readings_front.h"
#include "SAB_readings_rear.h"
#include "EM_measurement.h"
#include "EM_status.h"
#include "IMU_accelerometer.h"
#include "IMU_gryoscope.h"
#endif
