#define ID_MC_TEMPERATURES_1										0xA0
#define ID_MC_TEMPERATURES_2										0xA1
#define ID_MC_TEMPERATURES_3										0xA2
#define ID_MC_ANALOG_INPUTS_VOLTAGES								0xA3
#define ID_MC_DIGITAL_INPUT_STATUS									0xA4
#define ID_MC_MOTOR_POSITION_INFORMATION							0xA5
#define ID_MC_CURRENT_INFORMATION									0xA6
#define ID_MC_VOLTAGE_INFORMATION									0xA7
#define ID_MC_FLUX_INFORMATION										0xA8
#define ID_MC_INTERNAL_VOLTAGES										0xA9
#define ID_MC_INTERNAL_STATES										0xAA
#define ID_MC_FAULT_CODES											0xAB
#define ID_MC_TORQUE_TIMER_INFORMATION								0xAC
#define ID_MC_FLUX_WEAKENING_OUTPUT									0xAD
#define ID_MC_FIRMWARE_INFORMATION									0xAE
#define ID_MC_DIAGNOSTIC_DATA										0xAF

#define ID_MC_COMMAND_MESSAGE										0xC0
#define ID_MC_READ_WRITE_PARAMETER_COMMAND							0xC1
#define ID_MC_READ_WRITE_PARAMETER_RESPONSE							0xC2
#define ID_MCU_STATUS												0xC3
#define ID_MCU_PEDAL_READINGS										0xC4
#define ID_MCU_ANALOG_READINGS										0xCC

#define ID_BMS_ONBOARD_TEMPERATURES									0xD5
#define ID_BMS_ONBOARD_DETAILED_TEMPERATURES						0xD6 // TODO rename to bms_detailed_onboard_temperatures when we're not in the middle of a development cycle
#define ID_BMS_VOLTAGES												0xD7
#define ID_BMS_DETAILED_VOLTAGES									0xD8
#define ID_BMS_TEMPERATURES											0xD9
#define ID_BMS_DETAILED_TEMPERATURES								0xDA
#define ID_BMS_STATUS												0xDB
#define ID_FH_WATCHDOG_TEST											0xDC
#define ID_CCU_STATUS												0xDD
#define ID_BMS_BALANCING_STATUS										0xDE // TODO rename to bms_balancing_cells when we're not in the middle of a development cycle

#define ID_BMS_READ_WRITE_PARAMETER_COMMAND							0xE0 // TODO define this message
#define ID_BMS_PARAMETER_RESPONSE									0xE1 // TODO define this message
#define ID_BMS_COULOMB_COUNTS										0xE2
#define ID_MCU_GPS_READINGS											0xE7
#define ID_MCU_WHEEL_SPEED									    	0xEA
#define ID_DASHBOARD_STATUS                                         0xEB
#define ID_SAB_READINGS_FRONT                                       0xEC
#define ID_SAB_READINGS_REAR                                        0xED
#define ID_SAB_READINGS_GPS                                         0xEE

#define ID_EM_MEASUREMENT                                           0x100
#define ID_EM_STATUS                                                0x400

#define ID_IMU_ACCELEROMETER                                        0x470
#define ID_IMU_GYROSCOPE                                            0x471
