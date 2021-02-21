/*
 * Teensy 3.5 Telemetry Control Unit code
 * Written by Soohyun Kim, with assistance by Ryan Gallaway and Nathan Cheek. 
 * 
 * Rev 2 - 4/23/2019
 */

#define HT_DEBUG_EN // allows use of CAN message print() commands

#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <Wire.h>
#include <Metro.h>
#include <Adafruit_GPS.h>
#include <Time.h>

#include "SDUtils.h"
#include "XBUtils.h"

FlexCAN CAN(500000);
File logfile;

Adafruit_GPS GPS(&Serial1);

MCU_status mcu_status;
MCU_pedal_readings mcu_pedal_readings;
GLV_current_readings current_readings;
BMS_voltages bms_voltages;
BMS_detailed_voltages bms_detailed_voltages[8][3];
BMS_temperatures bms_temperatures;
BMS_detailed_temperatures bms_detailed_temperatures[8];
BMS_onboard_temperatures bms_onboard_temperatures;
BMS_onboard_detailed_temperatures bms_onboard_detailed_temperatures[8];
BMS_status bms_status;
BMS_balancing_status bms_balancing_status[2];
BMS_coulomb_counts bms_coulomb_counts;
CCU_status ccu_status;
MC_temperatures_1 mc_temperatures_1;
MC_temperatures_2 mc_temperatures_2;
MC_temperatures_3 mc_temperatures_3;
//MC_analog_input_voltages mc_analog_input_voltages;
//MC_digital_input_status mc_digital_input_status;
MC_motor_position_information mc_motor_position_information;
MC_current_information mc_current_information;
MC_voltage_information mc_voltage_information;
MC_internal_states mc_internal_states;
MC_fault_codes mc_fault_codes;
MC_torque_timer_information mc_torque_timer_information;
MC_modulation_index_flux_weakening_output_information mc_modulation_index_flux_weakening_output_information;
MC_firmware_information mc_firmware_information;
MC_command_message mc_command_message;
MC_read_write_parameter_command mc_read_write_parameter_command;
MC_read_write_parameter_response mc_read_write_parameter_response;
FCU_accelerometer_values fcu_accelerometer_values;
MCU_GPS_readings mcu_gps_readings;
TCU_wheel_rpm tcu_wheel_rpm_front;
TCU_wheel_rpm tcu_wheel_rpm_rear;
TCU_distance_traveled tcu_distance_traveled;

void setup() {
	// Real-time clock
	//Teensy3Clock.set(9999999999); // set time (epoch) at powerup (COMMENT OUT THIS LINE AND PUSH ONCE RTC HAS BEEN SET!!!!)
	setSyncProvider((getExternalTime) teensy3_clock_class::get); // registers Teensy RTC as system time
	Serial.println(timeStatus() == timeSet ? "System time set to RTC" : "RTC not set up - uncomment Teensy3Clock.set() to set time");

	// Serial / XBee / CAN 
	Serial.begin(115200);
	XB.begin(115200);
	FLEXCAN0_MCR &= 0xFFFDFFFF; // Enables CAN message self-reception
	CAN.begin();

	// GPS
	GPS.begin(9600);
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); // specify data to be received (minimum + fix)
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ); // set update rate (10Hz)
	GPS.sendCommand(PGCMD_ANTENNA); // report data about antenna

	// SD Card
	Serial.println("Initializing SD card...");
	SdFile::dateTimeCallback(HT::SDUtil::datetime_callback); // Set date/time callback function
	Serial.println(HT::SDUtil::initFD(logfile) ? "Successfully opened SD file" : "Failed to open SD file");

	logfile.println("time,msg.id,msg.len,data"); // Print CSV heading to the logfile
	logfile.flush();
}

void loop() {
	parse_can_message();
	check_xbee_timers();

    static Metro timer_flush = Metro(1000);
	if (timer_flush.check()) // Occasionally flush SD buffer (max one second data loss after power-off)
		logfile.flush();

	// GPS
    static Metro timer_gps = Metro(100);
	static bool pending_gps_data = false;
	GPS.read();
	if (GPS.newNMEAreceived()) {
		GPS.parse(GPS.lastNMEA());
		pending_gps_data = true;
	}
	if (timer_gps.check() && pending_gps_data) {
		pending_gps_data = false;
        mcu_gps_readings.set_latitude(GPS.latitude_fixed);
        mcu_gps_readings.set_longitude(GPS.longitude_fixed);
        CAN.write(HT::XBUtil::write(ID_MCU_GPS_READINGS, mcu_gps_readings));
	}
}

void parse_can_message() {
    CAN_message_t msg_rx;
	union {
		BMS_detailed_voltages bdv;
		BMS_onboard_detailed_temperatures bodt;
		BMS_detailed_temperatures bdt;
		BMS_balancing_status bbs;
	} temp;

	while (CAN.read(msg_rx)) {
		HT::SDUtil::write(logfile, msg_rx); // Write to SD card buffer (if the buffer fills up, triggering a flush to disk, this will take 8ms)

		// Identify received CAN messages and load contents into corresponding structs
		switch(msg_rx.id) {
#define loadcase(id, msg, ...) case id: msg.load(msg_rx.buf); __VA_ARGS__; break;
		loadcase(ID_MCU_STATUS, 						mcu_status);
		loadcase(ID_MCU_STATUS,							mcu_status);
		loadcase(ID_MCU_PEDAL_READINGS,					mcu_pedal_readings);
		loadcase(ID_GLV_CURRENT_READINGS,				current_readings);
		loadcase(ID_BMS_VOLTAGES,						bms_voltages);
		loadcase(ID_BMS_TEMPERATURES,					bms_temperatures);
		loadcase(ID_BMS_ONBOARD_TEMPERATURES,			bms_onboard_temperatures);
		loadcase(ID_BMS_STATUS,							bms_status);
		loadcase(ID_BMS_COULOMB_COUNTS,					bms_coulomb_counts);
		loadcase(ID_CCU_STATUS,							ccu_status);
		loadcase(ID_MC_TEMPERATURES_1,					mc_temperatures_1);
		loadcase(ID_MC_TEMPERATURES_2,					mc_temperatures_2);
		loadcase(ID_MC_TEMPERATURES_3,					mc_temperatures_3);
		// loadcase(ID_MC_ANALOG_INPUTS_VOLTAGES,		mc_analog_input_voltages);
		// loadcase(ID_MC_DIGITAL_INPUT_STATUS,			mc_digital_input_status);
		loadcase(ID_MC_MOTOR_POSITION_INFORMATION,		mc_motor_position_information);
		loadcase(ID_MC_CURRENT_INFORMATION,				mc_current_information);
		loadcase(ID_MC_VOLTAGE_INFORMATION,				mc_voltage_information);
		loadcase(ID_MC_INTERNAL_STATES,					mc_internal_states);
		loadcase(ID_MC_FAULT_CODES,						mc_fault_codes);
		loadcase(ID_MC_TORQUE_TIMER_INFORMATION,		mc_torque_timer_information);
		loadcase(ID_MC_FIRMWARE_INFORMATION,			mc_firmware_information);
		loadcase(ID_MC_COMMAND_MESSAGE,					mc_command_message);
		loadcase(ID_MC_READ_WRITE_PARAMETER_COMMAND,	mc_read_write_parameter_command);
		loadcase(ID_MC_READ_WRITE_PARAMETER_RESPONSE,	mc_read_write_parameter_response);
		loadcase(ID_FCU_ACCELEROMETER,					fcu_accelerometer_values);
		loadcase(ID_TCU_WHEEL_RPM_REAR,					tcu_wheel_rpm_rear);
		loadcase(ID_TCU_WHEEL_RPM_FRONT,				tcu_wheel_rpm_front);
		loadcase(ID_TCU_DISTANCE_TRAVELED,				tcu_distance_traveled);
		loadcase(ID_BMS_DETAILED_VOLTAGES, 				temp.bdv, 	bms_detailed_voltages[temp.bdv.get_ic_id()][temp.bdv.get_group_id].load(msg_rx.buf));
		loadcase(ID_BMS_DETAILED_TEMPERATURES, 			temp.bdt, 	bms_detailed_temperatures[temp.bdt.get_ic_id()].load(msg_rx.buf));
		loadcase(ID_BMS_ONBOARD_DETAILED_TEMPERATURES,	temp.bodt, 	bms_onboard_detailed_temperatures[temp.bodt.get_ic_id()].load(msg_rx.buf));
		loadcase(ID_BMS_BALANCING_STATUS, 				temp.bbs, 	bms_balancing_status[temp.bdv.get_group_id].load(msg_rx.buf));
		loadcase(ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION, mc_modulation_index_flux_weakening_output_information);
#undef loadcase
		}
	}
}

void check_xbee_timers() {

    static Metro xb_metro_100ms = Metro(100);
    if (xb_metro_100ms.check()) {
		HT::XBUtil::write(ID_MC_MOTOR_POSITION_INFORMATION, 	mc_motor_position_information);
		HT::XBUtil::write(ID_MC_CURRENT_INFORMATION, 			mc_current_information);
		HT::XBUtil::write(ID_MC_VOLTAGE_INFORMATION, 			mc_voltage_information);
	}

	static Metro xb_metro_200ms = Metro(200);
    if (xb_metro_200ms.check()) {
		HT::XBUtil::write(ID_MC_COMMAND_MESSAGE,				mc_command_message);
		HT::XBUtil::write(ID_MC_TORQUE_TIMER_INFORMATION,		mc_torque_timer_information);
		HT::XBUtil::write(ID_MCU_PEDAL_READINGS,				mcu_pedal_readings);
		HT::XBUtil::write(ID_TCU_WHEEL_RPM_REAR,				tcu_wheel_rpm_rear);
		HT::XBUtil::write(ID_TCU_WHEEL_RPM_FRONT,				tcu_wheel_rpm_front);
		HT::XBUtil::write(ID_TCU_DISTANCE_TRAVELED,				tcu_distance_traveled);
	}
	
	static Metro xb_metro_1s = Metro(1000);
    if (xb_metro_1s.check()) {
		HT::XBUtil::write(ID_BMS_VOLTAGES,						bms_voltages );
		HT::XBUtil::write(ID_BMS_STATUS,						bms_status );
		HT::XBUtil::write(ID_BMS_COULOMB_COUNTS,				bms_coulomb_counts );
	}

	static Metro xb_metro_2s = Metro(2000);
    if (xb_metro_2s.check()) {
		HT::XBUtil::write(ID_MC_INTERNAL_STATES,				mc_internal_states);
		HT::XBUtil::write(ID_MC_FAULT_CODES,					mc_fault_codes);
		HT::XBUtil::write(ID_MCU_STATUS,						mcu_status);
	}

	static Metro xb_metro_3s = Metro(3000);
    if (xb_metro_3s.check()) {
		HT::XBUtil::write(ID_MC_TEMPERATURES_1, 				mc_temperatures_1);
		HT::XBUtil::write(ID_MC_TEMPERATURES_2, 				mc_temperatures_2);
		HT::XBUtil::write(ID_MC_TEMPERATURES_3, 				mc_temperatures_3);
		HT::XBUtil::write(ID_BMS_TEMPERATURES, 					bms_temperatures);

		for (int ic = 0; ic < 8; ic++)
			for (int group = 0; group < 3; group++)
				HT::XBUtil::write(ID_BMS_DETAILED_VOLTAGES, 	bms_detailed_voltages[ic][group]);
		for (int ic = 0; ic < 8; ic++)
			HT::XBUtil::write(ID_BMS_DETAILED_TEMPERATURES, 	bms_detailed_temperatures[ic]);
		for (int i = 0; i < 2; i++)
			HT::XBUtil::write(ID_BMS_BALANCING_STATUS, 			bms_balancing_status[i]);
	}
}