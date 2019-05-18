/*
 * Teensy 3.5 Telemetry Control Unit code
 * Written by Soohyun Kim, with assistance by Ryan Gallaway and Nathan Cheek. 
 * 
 * Rev 2 - 4/23/2019
 */

#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <ADC_SPI.h>
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Wire.h>
#include <TimeLib.h>
#include <Metro.h>
#include <XBTools.h>
#include <Adafruit_GPS.h>

#define XB Serial2
#define XBEE_PKT_LEN 15

#define BMS_HIGH 134 // ~3V on BMS_OK line
#define IMD_HIGH 134 // ~3V on OKHS line

FlexCAN CAN(500000);
static CAN_message_t msg_rx;
static CAN_message_t msg_tx;
static CAN_message_t msg_log;
static CAN_message_t xb_msg;
File logger;

ADC_SPI ADC(10);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_GPS GPS(&Serial1);

Metro timer_debug_mcu_status = Metro(2000);
Metro timer_debug_mcu_pedal_readings = Metro(200);
Metro timer_debug_bms_balancing_status = Metro(3000);
Metro timer_accelerometer = Metro(100);
Metro timer_current = Metro(500);
Metro timer_debug_bms_status = Metro(1000);
Metro timer_debug_bms_temperatures = Metro(3000);
Metro timer_debug_bms_detailed_temperatures = Metro(3000);
Metro timer_debug_bms_voltages = Metro(1000);
Metro timer_debug_bms_detailed_voltages = Metro(3000);
Metro timer_debug_rms_command_message = Metro(200);
Metro timer_debug_rms_current_information = Metro(100);
Metro timer_debug_rms_fault_codes = Metro(2000);
Metro timer_debug_rms_internal_states = Metro(2000);
Metro timer_debug_rms_motor_position_information = Metro(100);
Metro timer_debug_rms_temperatures_1 = Metro(3000);
Metro timer_debug_rms_temperatures_3 = Metro(3000);
Metro timer_debug_rms_torque_timer_information = Metro(200);
Metro timer_debug_rms_voltage_information = Metro(100);
Metro timer_detailed_voltages = Metro(1000);
Metro timer_status_send = Metro(100);
Metro timer_status_send_xbee = Metro(2000);
Metro gpsTimer = Metro(500);
Metro timer_debug_RTC = Metro(1000);

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

// data for gps
static int latitudex10000;
static int longitudex10000;
static int altitudex10000;
static int speedx10000;



// flags double in function as timestamps
static int flag_mcu_status;
static int flag_mcu_pedal_readings;
static int flag_current_readings;
static int flag_bms_voltages;
static int flag_bms_detailed_voltages;
static int flag_bms_temperatures;
static int flag_bms_detailed_temperatures;
static int flag_bms_onboard_temperatures;
static int flag_bms_onboard_detailed_temperatures;
static int flag_bms_status;
static int flag_bms_balancing_status;                                               
static int flag_bms_coulomb_counts;
static int flag_ccu_status;
static int flag_mc_temperatures_1;
static int flag_mc_temperatures_2;
static int flag_mc_temperatures_3;
//static int flag_mc_analog_input_voltages;
//static int flag_mc_digital_input_status;
static int flag_mc_motor_position_information;
static int flag_mc_current_information;
static int flag_mc_voltage_information;
static int flag_mc_internal_states;
static int flag_mc_fault_codes;
static int flag_mc_torque_timer_information;
static int flag_mc_modulation_index_flux_weakening_output_information;
static int flag_mc_firmware_information;
static int flag_mc_command_message;
static int flag_mc_read_write_parameter_command;
static int flag_mc_read_write_parameter_response;
static int flag_fcu_accelerometer_values;
static int flag_gps_alpha;
static int flag_gps_beta;

void setup() {
  
  
      // Teensy 3.5 code
  NVIC_ENABLE_IRQ(IRQ_CAN0_MESSAGE);                                   // Enables interrupts on the teensy for CAN messages
  attachInterruptVector(IRQ_CAN0_MESSAGE, parse_can_message);          // Attaches parse_can_message() as ISR
  CAN0_IMASK1 |= 0x00000020;                                           // Allows "CAN message arrived" bit in flag register to throw interrupt
  CAN0_MCR &= 0xFFFDFFFF;                                              // Enables CAN message self-reception
  
  /*      // Teensy 3.2 code
  NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);                                   // Enables interrupts on the teensy for CAN messages
  attachInterruptVector(IRQ_CAN_MESSAGE, parse_can_message);          // Attaches parse_can_message() as ISR
  CAN0_IMASK1 |= 0x00000020;
  CAN0_MCR &= 0xFFFDFFFF;
  */
  
  pinMode(10, OUTPUT);                                                // Initialize pin 10 as output; this is necessary for the SD Library
  Serial.begin(115200);
  CAN.begin();
  Serial.println("Initializing SD card...");
  //SD.begin(10);                                                     // Begin Arduino SD API (3.2)
  if (!SD.begin(BUILTIN_SDCARD)) {                                    // Begin Arduino SD API (3.5)
    Serial.println("SD card failed, or not present");
  }
  logger = SD.open("datalog.csv", FILE_WRITE);                      // Open file for writing.
  if (logger) {
    Serial.println("Successfully opened SD file");
  } else {
    Serial.println("Failed to open SD file");
  }
  logger.println("time,msg.id,msg.len,data");                         // Print heading to the file.
  logger.flush();

  //Teensy3Clock.set(9999999999);                                     // set time (epoch) at powerup  (COMMENT OUT THIS LINE AND PUSH ONCE RTC HAS BEEN SET!!!!)
  setSyncProvider(getTeensy3Time);                                    // registers Teensy RTC as system time

  setupAccelerometer();

  XB.begin(115200);

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);                       // specify data to be received (minimum + fix)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);                          // set update rate (1Hz)
  GPS.sendCommand(PGCMD_ANTENNA);                                     // report data about antenna

  pinMode(A12, INPUT);
  pinMode(A13, INPUT);
}

void loop() {
  process_SD();

  send_xbee();
    
  if (timer_debug_RTC.check()) {
    Serial.println(Teensy3Clock.get());
  }
    
  if (timer_accelerometer.check()) {
      processAccelerometer(); 
  }
  if (timer_current.check()) {
      
    //self derived
    double current_ecu = ((double)(analogRead(A13)-96))*0.029412;
    double current_cooling = ((double)(analogRead(A12)-96))*0.029412;
    //Serial.println(current_cooling);
    //Serial.println(current_ecu);
      
    current_readings.set_ecu_current_value((short)((int)(current_ecu*100)));
    current_readings.set_cooling_current_value((short)((int)(current_cooling*100)));

    // order of bytes of each value is in reverse: buf[1],buf[0] is x value, buf[3],buf[2] is y value, and etc.
    current_readings.write(msg_tx.buf);
    msg_tx.id = ID_GLV_CURRENT_READINGS;
    msg_tx.len = sizeof(CAN_message_glv_current_readings_t);
    CAN.write(msg_tx);

    current_readings.write(xb_msg.buf);
    xb_msg.id = ID_GLV_CURRENT_READINGS;
    xb_msg.len = sizeof(CAN_message_glv_current_readings_t);
    write_xbee_data();     
  }
    
  GPS.read();
  if (GPS.newNMEAreceived()) {
    GPS.parse(GPS.lastNMEA());
  }
  if (gpsTimer.check()) {
    msg_tx.id = ID_ECU_GPS_READINGS_ALPHA;
    msg_tx.len = 8;
    int latitudex10000_send = (int)(GPS.latitude*10000);
    int longitudex10000_send = (int)(GPS.longitude*10000);
    //Serial.print("Latitude (x10000): ");
    //Serial.println(latitudex10000);
    //Serial.print("Longitude (x10000): ");
    //Serial.println(longitudex10000);
    memcpy(&(msg_tx.buf[0]), &latitudex10000_send, sizeof(int));
    memcpy(&(msg_tx.buf[4]), &longitudex10000_send, sizeof(int));
    CAN.write(msg_tx);

    msg_tx.id = ID_ECU_GPS_READINGS_BETA;
    msg_tx.len = 8;
    int altitudex10000_send = (int)(GPS.altitude*10000);
    //Serial.print("Altitude (x10000): ");
    //Serial.println(altitudex10000);
    int speedx10000_send = (int)(GPS.speed*10000);
    //Serial.print("Speed (x10000): ");
    //Serial.println(speedx10000);
    memcpy(&(msg_tx.buf[0]), &altitudex10000_send, sizeof(int));
    memcpy(&(msg_tx.buf[4]), &speedx10000_send, sizeof(int));
    CAN.write(msg_tx);    
  }
}

void process_SD() {
  // check flags and write to SD therefrom
  if (flag_mcu_status) {
    mcu_status.write(msg_log.buf);
    msg_log.id = ID_MCU_STATUS;
    msg_log.len = sizeof(CAN_message_mcu_status_t);
    write_to_SD(flag_mcu_status);
  }
  if (flag_mcu_pedal_readings) {
    mcu_pedal_readings.write(msg_log.buf);
    msg_log.id = ID_MCU_PEDAL_READINGS;
    msg_log.len = sizeof(CAN_message_mcu_pedal_readings_t);
    write_to_SD(flag_mcu_pedal_readings);
  }
  
  if (flag_current_readings) {
    current_readings.write(msg_log.buf);
    msg_log.id = ID_GLV_CURRENT_READINGS;
    msg_log.len = sizeof(CAN_message_glv_current_readings_t);
    write_to_SD(flag_current_readings);
  }

  if (flag_bms_voltages) {
    bms_voltages.write(msg_log.buf);
    msg_log.id = ID_BMS_VOLTAGES;
    msg_log.len = sizeof(CAN_message_bms_voltages_t);
    write_to_SD(flag_bms_voltages);
  }

  if (flag_bms_detailed_voltages) {
    for (int ic = 0; ic < 8; ic++) {
      for (int group = 0; group < 3; group++) {
        bms_detailed_voltages[ic][group].write(msg_log.buf);
        msg_log.id = ID_BMS_DETAILED_VOLTAGES;
        msg_log.len = sizeof(CAN_message_bms_detailed_voltages_t);
        write_to_SD(flag_bms_detailed_voltages);
      }
    } 
  }

  if (flag_bms_temperatures) {
    bms_temperatures.write(msg_log.buf);
    msg_log.id = ID_BMS_TEMPERATURES;
    msg_log.len = sizeof(CAN_message_bms_temperatures_t);
    write_to_SD(flag_bms_temperatures);
  }

  if (flag_bms_detailed_temperatures) {
    for (int ic = 0; ic < 8; ic++) {
      bms_detailed_temperatures[ic].write(msg_log.buf);
      msg_log.id = ID_BMS_DETAILED_TEMPERATURES;
      msg_log.len = sizeof(CAN_message_bms_detailed_temperatures_t);
      write_to_SD(flag_bms_detailed_temperatures);
    }
  }

  if (flag_bms_onboard_temperatures) {
    bms_onboard_temperatures.write(msg_log.buf);
    msg_log.id = ID_BMS_ONBOARD_TEMPERATURES;
    msg_log.len = sizeof(CAN_message_bms_onboard_temperatures_t);
    write_to_SD(flag_bms_onboard_temperatures);
  }

  if (flag_bms_onboard_detailed_temperatures) {
    for (int i=0; i<8; i++) {
      bms_onboard_detailed_temperatures[i].write(msg_log.buf);
      msg_log.id = ID_BMS_ONBOARD_DETAILED_TEMPERATURES;
      msg_log.len = sizeof(CAN_message_bms_onboard_detailed_temperatures_t);
      write_to_SD(flag_bms_onboard_detailed_temperatures);
    }
  }

  if (flag_bms_status) {
    bms_status.write(msg_log.buf);
    msg_log.id = ID_BMS_STATUS;
    msg_log.len = sizeof(CAN_message_bms_status_t);
    write_to_SD(flag_bms_status);
  }
  
  if (flag_bms_balancing_status) {
    for (int i=0; i<2; i++) {
      bms_balancing_status[i].write(msg_log.buf);
      msg_log.id = ID_BMS_BALANCING_STATUS;
      msg_log.len = sizeof(CAN_message_bms_balancing_status_t);
      write_to_SD(flag_bms_balancing_status);
    }
  }

  if (flag_bms_coulomb_counts) {
    bms_coulomb_counts.write(msg_log.buf);
    msg_log.id = ID_BMS_COULOMB_COUNTS;
    msg_log.len = sizeof(CAN_message_bms_coulomb_counts_t);
    write_to_SD(flag_bms_coulomb_counts);
  }
  
  if (flag_ccu_status) {
    ccu_status.write(msg_log.buf);
    msg_log.id = ID_CCU_STATUS;
    msg_log.len = sizeof(CAN_message_ccu_status_t);
    write_to_SD(flag_ccu_status);
  }

  if (flag_mc_temperatures_1) {
    mc_temperatures_1.write(msg_log.buf);
    msg_log.id = ID_MC_TEMPERATURES_1;
    msg_log.len = sizeof(CAN_message_mc_temperatures_1_t);
    write_to_SD(flag_mc_temperatures_1);
  }

  if (flag_mc_temperatures_2) {
    mc_temperatures_2.write(msg_log.buf);
    msg_log.id = ID_MC_TEMPERATURES_2;
    msg_log.len = sizeof(CAN_message_mc_temperatures_2_t);
    write_to_SD(flag_mc_temperatures_2);
  }

  if (flag_mc_temperatures_3) {
    mc_temperatures_3.write(msg_log.buf);
    msg_log.id = ID_MC_TEMPERATURES_3;
    msg_log.len = sizeof(CAN_message_mc_temperatures_3_t);
    write_to_SD(flag_mc_temperatures_3);
  }
  /*
  if (flag_mc_analog_input_voltages) {
    mc_analog_input_voltages.write(msg_log.buf);
    msg_log.id = ID_MC_ANALOG_INPUTS_VOLTAGES;
    msg_log.len = sizeof(CAN_message_mc_analog_input_voltages_t);
    write_to_SD(flag_mc_analog_input_voltages);
  }

  if (flag_mc_digital_input_status) {
    mc_digital_input_status.write(msg_log.buf);
    msg_log.id = ID_MC_DIGITAL_INPUT_STATUS;
    msg_log.len = sizeof(CAN_message_mc_digital_input_status_t);
    write_to_SD(flag_mc_digital_input_status);
  }
  */
  if (flag_mc_motor_position_information) {
    mc_motor_position_information.write(msg_log.buf);
    msg_log.id = ID_MC_MOTOR_POSITION_INFORMATION;
    msg_log.len = sizeof(CAN_message_mc_motor_position_information_t);
    write_to_SD(flag_mc_motor_position_information);
  }

  if (flag_mc_current_information) {
    mc_current_information.write(msg_log.buf);
    msg_log.id = ID_MC_CURRENT_INFORMATION;
    msg_log.len = sizeof(CAN_message_mc_current_information_t);
    write_to_SD(flag_mc_current_information);
  }

  if (flag_mc_voltage_information) {
    mc_voltage_information.write(msg_log.buf);
    msg_log.id = ID_MC_VOLTAGE_INFORMATION;
    msg_log.len = sizeof(CAN_message_mc_voltage_information_t);
    write_to_SD(flag_mc_voltage_information);
  }

  if (flag_mc_internal_states) {
    mc_internal_states.write(msg_log.buf);
    msg_log.id = ID_MC_INTERNAL_STATES;
    msg_log.len = sizeof(CAN_message_mc_internal_states_t);
    write_to_SD(flag_mc_internal_states);
  }

  if (flag_mc_fault_codes) {
    mc_fault_codes.write(msg_log.buf);
    msg_log.id = ID_MC_FAULT_CODES;
    msg_log.len = sizeof(CAN_message_mc_fault_codes_t);
    write_to_SD(flag_mc_fault_codes);
  }

  if (flag_mc_torque_timer_information) {
    mc_torque_timer_information.write(msg_log.buf);
    msg_log.id = ID_MC_TORQUE_TIMER_INFORMATION;
    msg_log.len = sizeof(CAN_message_mc_torque_timer_information_t);
    write_to_SD(flag_mc_torque_timer_information);
  }

  if (flag_mc_modulation_index_flux_weakening_output_information) {
    mc_modulation_index_flux_weakening_output_information.write(msg_log.buf);
    msg_log.id = ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION;
    msg_log.len = sizeof(CAN_message_mc_modulation_index_flux_weakening_output_information_t);
    write_to_SD(flag_mc_modulation_index_flux_weakening_output_information);
  }

  if (flag_mc_firmware_information) {
    mc_firmware_information.write(msg_log.buf);
    msg_log.id = ID_MC_FIRMWARE_INFORMATION;
    msg_log.len = sizeof(CAN_message_mc_firmware_information_t);
    write_to_SD(flag_mc_firmware_information);
  }

  if (flag_mc_command_message) {
    mc_command_message.write(msg_log.buf);
    msg_log.id = ID_MC_COMMAND_MESSAGE;
    msg_log.len = sizeof(CAN_message_mc_command_message_t);
    write_to_SD(flag_mc_command_message);
  }

  if (flag_mc_read_write_parameter_command) {
    mc_read_write_parameter_command.write(msg_log.buf);
    msg_log.id = ID_MC_READ_WRITE_PARAMETER_COMMAND;
    msg_log.len = sizeof(CAN_message_mc_read_write_parameter_command_t);
    write_to_SD(flag_mc_read_write_parameter_command);
  }

  if (flag_mc_read_write_parameter_response) {
    mc_read_write_parameter_response.write(msg_log.buf);
    msg_log.id = ID_MC_READ_WRITE_PARAMETER_RESPONSE;
    msg_log.len = sizeof(CAN_message_mc_read_write_parameter_response_t);
    write_to_SD(flag_mc_read_write_parameter_response);
  }

  if (flag_fcu_accelerometer_values) {
    fcu_accelerometer_values.write(msg_log.buf);
    msg_log.id = ID_FCU_ACCELEROMETER;
    msg_log.len = sizeof(CAN_message_fcu_accelerometer_values_t);
    write_to_SD(flag_fcu_accelerometer_values);
  }
  if (flag_gps_alpha) {
    memcpy(&(msg_log.buf[0]), &latitudex10000, sizeof(int));
    memcpy(&(msg_log.buf[4]), &longitudex10000, sizeof(int));
    msg_log.id = ID_ECU_GPS_READINGS_ALPHA;
    // TODO msg_log.len = sizeof(CAN_message_gps_alpha_t);
    msg_log.len = 8;
    write_to_SD(flag_gps_alpha);
  }
  if (flag_gps_beta) {
    memcpy(&(msg_log.buf[0]), &altitudex10000, sizeof(int));
    memcpy(&(msg_log.buf[4]), &speedx10000, sizeof(int));
    msg_log.id = ID_ECU_GPS_READINGS_BETA;
    // TODO msg_log.len = sizeof(CAN_message_gps_beta_t);
    msg_log.len = 8;
    write_to_SD(flag_gps_beta);
  }
}

void parse_can_message() {

  // identify received CAN messages and load contents into corresponding structs
  while (CAN.read(msg_rx)) {
    //Serial.println("Received!");
    int time_now = Teensy3Clock.get();                                          // RTC!!
    if (msg_rx.id == ID_MCU_STATUS) {
      mcu_status.load(msg_rx.buf);
      flag_mcu_status = time_now;
    }
    if (msg_rx.id == ID_MCU_PEDAL_READINGS) {
      mcu_pedal_readings.load(msg_rx.buf);
      flag_mcu_pedal_readings = time_now;
    }
    if (msg_rx.id == ID_GLV_CURRENT_READINGS) {
      current_readings.load(msg_rx.buf);
      flag_current_readings = time_now;
    }
    if (msg_rx.id == ID_BMS_VOLTAGES) {
      bms_voltages.load(msg_rx.buf);
      flag_bms_voltages = time_now;
    }
    if (msg_rx.id == ID_BMS_DETAILED_VOLTAGES) {
      BMS_detailed_voltages temp = BMS_detailed_voltages(msg_rx.buf);
      bms_detailed_voltages[temp.get_ic_id()][temp.get_group_id()].load(msg_rx.buf);
      flag_bms_detailed_voltages = time_now;
    }
    if (msg_rx.id == ID_BMS_TEMPERATURES) {
      bms_temperatures.load(msg_rx.buf);
      flag_bms_temperatures = time_now;
    }
    if (msg_rx.id == ID_BMS_DETAILED_TEMPERATURES) {
      BMS_detailed_temperatures temp = BMS_detailed_temperatures(msg_rx.buf);
      bms_detailed_temperatures[temp.get_ic_id()].load(msg_rx.buf);
      flag_bms_detailed_temperatures = time_now;
    }
    if (msg_rx.id == ID_BMS_ONBOARD_TEMPERATURES) {
      bms_onboard_temperatures.load(msg_rx.buf);
      flag_bms_onboard_temperatures = time_now;
    }
    if (msg_rx.id == ID_BMS_ONBOARD_DETAILED_TEMPERATURES) {
      BMS_onboard_detailed_temperatures temp = BMS_onboard_detailed_temperatures(msg_rx.buf);
      bms_onboard_detailed_temperatures[temp.get_ic_id()].load(msg_rx.buf);
      flag_bms_onboard_detailed_temperatures = time_now;
    }
    if (msg_rx.id == ID_BMS_STATUS) {
      bms_status.load(msg_rx.buf);
      flag_bms_status = time_now;
    }
    if (msg_rx.id == ID_BMS_BALANCING_STATUS) {
      BMS_balancing_status temp = BMS_balancing_status(msg_rx.buf);
      bms_balancing_status[temp.get_group_id()].load(msg_rx.buf);
      flag_bms_balancing_status = time_now;
    }
    if (msg_rx.id == ID_BMS_COULOMB_COUNTS) {
      bms_coulomb_counts.load(msg_rx.buf);
      flag_bms_coulomb_counts = time_now;
    }
    if (msg_rx.id == ID_CCU_STATUS) {
      ccu_status.load(msg_rx.buf);
      flag_ccu_status = time_now;
    }
    if (msg_rx.id == ID_MC_TEMPERATURES_1) {
      mc_temperatures_1.load(msg_rx.buf);
      flag_mc_temperatures_1 = time_now;
    }
    if (msg_rx.id == ID_MC_TEMPERATURES_2) {
      mc_temperatures_2.load(msg_rx.buf);
      flag_mc_temperatures_2 = time_now;
    }
    if (msg_rx.id == ID_MC_TEMPERATURES_3) {
      mc_temperatures_3.load(msg_rx.buf);
      flag_mc_temperatures_3 = time_now;
    }
    /*
    if (msg_rx.id == ID_MC_ANALOG_INPUTS_VOLTAGES) {
      mc_analog_input_voltages.load(msg_rx.buf);
      flag_mc_analog_input_voltages = time_now;
    }
    if (msg_rx.id == ID_MC_DIGITAL_INPUT_STATUS) {
      mc_digital_input_status.load(msg_rx.buf);
      flag_mc_digital_input_status = time_now;
    }
    */
    if (msg_rx.id == ID_MC_MOTOR_POSITION_INFORMATION) {
      mc_motor_position_information.load(msg_rx.buf);
      flag_mc_motor_position_information = time_now;
    }
    if (msg_rx.id == ID_MC_CURRENT_INFORMATION) {
      mc_current_information.load(msg_rx.buf);
      flag_mc_current_information = time_now;
    }
    if (msg_rx.id == ID_MC_VOLTAGE_INFORMATION) {
      mc_voltage_information.load(msg_rx.buf);
      flag_mc_voltage_information = time_now;
    }
    if (msg_rx.id == ID_MC_INTERNAL_STATES) {
      mc_internal_states.load(msg_rx.buf);
      flag_mc_internal_states = time_now;
    }
    if (msg_rx.id == ID_MC_FAULT_CODES) {
      mc_fault_codes.load(msg_rx.buf);
      flag_mc_fault_codes = time_now;
    }
    if (msg_rx.id == ID_MC_TORQUE_TIMER_INFORMATION) {
      mc_torque_timer_information.load(msg_rx.buf);
      flag_mc_torque_timer_information = time_now;
    }
    if (msg_rx.id == ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION) {
      mc_modulation_index_flux_weakening_output_information.load(msg_rx.buf);
      flag_mc_modulation_index_flux_weakening_output_information = time_now;
    }
    if (msg_rx.id == ID_MC_FIRMWARE_INFORMATION) {
      mc_firmware_information.load(msg_rx.buf);
      flag_mc_firmware_information = time_now;
    }
    if (msg_rx.id == ID_MC_COMMAND_MESSAGE) {
      mc_command_message.load(msg_rx.buf);
      flag_mc_command_message = time_now;
    }
    if (msg_rx.id == ID_MC_READ_WRITE_PARAMETER_COMMAND) {
      mc_read_write_parameter_command.load(msg_rx.buf);
      flag_mc_read_write_parameter_command = time_now;
    }
    if (msg_rx.id == ID_MC_READ_WRITE_PARAMETER_RESPONSE) {
      mc_read_write_parameter_response.load(msg_rx.buf);
      flag_mc_read_write_parameter_response = time_now;
    }
    if (msg_rx.id == ID_FCU_ACCELEROMETER) {
      fcu_accelerometer_values.load(msg_rx.buf);
      flag_fcu_accelerometer_values = time_now;
    }
    if (msg_rx.id == ID_ECU_GPS_READINGS_ALPHA) {
      memcpy(&latitudex10000, &(msg_rx.buf[0]), sizeof(int));
      memcpy(&longitudex10000, &(msg_rx.buf[4]), sizeof(int));
      flag_gps_alpha = time_now;
    }
    if (msg_rx.id == ID_ECU_GPS_READINGS_BETA) {
      memcpy(&altitudex10000, &(msg_rx.buf[0]), sizeof(int));
      memcpy(&speedx10000, &(msg_rx.buf[4]), sizeof(int));
      flag_gps_beta = time_now;
    }
  }
}

void write_to_SD(int& timestamp) {
  logger.print(timestamp);
  logger.print(",");
  logger.print(msg_log.id, HEX);
  logger.print(",");
  logger.print(msg_log.len);
  logger.print(",");
  for (int i=0; i<8; i++) {
    if (msg_log.buf[i]<16) {
      logger.print("0");
    }
    logger.print(msg_log.buf[i], HEX);
    logger.print("");
  }
  logger.println();
  logger.flush(); 

  // clear flag
  timestamp = 0;
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void setupAccelerometer() {
      
  // Initialise the sensor
  if(!accel.begin())
  {
    // There was a problem detecting the ADXL345 ... check your connections
    Serial.println("Sensor not detected!!!!!");
  }
  else {
    accel.setRange(ADXL345_RANGE_4_G);
  }
}

void processAccelerometer() {
  // Get a new sensor event
  sensors_event_t event; 
  accel.getEvent(&event);
  
  // Read accelerometer values into accelerometer struct
  fcu_accelerometer_values.set_values((short)((int)(event.acceleration.x*100)), (short)((int)(event.acceleration.y*100)), (short)((int)(event.acceleration.z*100)));
  
  // Send msg over CAN
  fcu_accelerometer_values.write(xb_msg.buf);
  xb_msg.id = ID_FCU_ACCELEROMETER;
  xb_msg.len = sizeof(CAN_message_fcu_accelerometer_values_t);
  write_xbee_data();
  
  fcu_accelerometer_values.write(msg_tx.buf);
  msg_tx.id = ID_FCU_ACCELEROMETER;
  msg_tx.len = sizeof(CAN_message_fcu_accelerometer_values_t);
  CAN.write(msg_tx);
  

  /*
  // order of bytes of each value is in reverse: buf[1],buf[0] is x value, buf[3],buf[2] is y value, and etc.
  Serial.println((short)((int)(event.acceleration.x*100)));
  Serial.print(msg.buf[1], HEX);
  Serial.println(msg.buf[0], HEX);
  */

  /*
  Serial.print("\n\nACCELEROMETER DATA\n\n");
  Serial.print(event.acceleration.x); Serial.print(", ");
  Serial.print(event.acceleration.y); Serial.print(", ");
  Serial.print(event.acceleration.z); Serial.println("\n\n");
  */
}

int write_xbee_data() {
    /*
     * DECODED FRAME STRUCTURE:
     * [ msg id (4) | msg len (1) | msg contents (8) | checksum (2) ]
     * ENCODED FRAME STRUCTURE:
     * [ fletcher (1) | msg id (4) | msg len (1) | msg contents (8) | checksum (2) | delimiter (1) ]
     */
    uint8_t xb_buf[XBEE_PKT_LEN];
    memcpy(xb_buf, &xb_msg.id, sizeof(xb_msg.id));        // msg id
    memcpy(xb_buf + sizeof(xb_msg.id), &xb_msg.len, sizeof(uint8_t));     // msg len
    memcpy(xb_buf + sizeof(xb_msg.id) + sizeof(uint8_t), xb_msg.buf, xb_msg.len); // msg contents

    // calculate checksum
    uint16_t checksum = fletcher16(xb_buf, XBEE_PKT_LEN - 2);
    //Serial.print("CHECKSUM: ");
    //Serial.println(checksum, HEX);
    memcpy(&xb_buf[XBEE_PKT_LEN - 2], &checksum, sizeof(uint16_t));

    for (int i = 0; i < XBEE_PKT_LEN; i++) {
      //Serial.print(xb_buf[i], HEX);
      //Serial.print(" ");
    }
    //Serial.println();

    uint8_t cobs_buf[2 + XBEE_PKT_LEN];
    cobs_encode(xb_buf, XBEE_PKT_LEN, cobs_buf);
    cobs_buf[XBEE_PKT_LEN+1] = 0x0;

    for (int i = 0; i < XBEE_PKT_LEN+2; i++) {
      //Serial.print(cobs_buf[i], HEX);
      //Serial.print(" ");
    }
    //Serial.println();

    int written = XB.write(cobs_buf, 2 + XBEE_PKT_LEN);
    //Serial.print("Wrote ");
    //Serial.print(written);
    //Serial.println(" bytes");

    memset(xb_buf, 0, sizeof(CAN_message_t));

    return written;
}

void send_xbee() {
    if (timer_debug_rms_temperatures_1.check()) {
        mc_temperatures_1.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_temperatures_1_t);
        xb_msg.id = ID_MC_TEMPERATURES_1;
        write_xbee_data();
        /*XB.print("MODULE A TEMP: ");
        XB.println(mc_temperatures_1.get_module_a_temperature() / (double) 10, 1);
        XB.print("MODULE B TEMP: ");
        XB.println(mc_temperatures_1.get_module_b_temperature() / (double) 10, 1);
        XB.print("MODULE C TEMP: ");
        XB.println(mc_temperatures_1.get_module_c_temperature() / (double) 10, 1);
        XB.print("GATE DRIVER BOARD TEMP: ");
        XB.println(mc_temperatures_1.get_gate_driver_board_temperature() / (double) 10, 1);*/
    }

    if (timer_debug_rms_temperatures_3.check()) {
        mc_temperatures_3.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_temperatures_3_t);
        xb_msg.id = ID_MC_TEMPERATURES_3;
        write_xbee_data();
        /*//XB.print("RTD 4 TEMP: "); // These aren't needed since we aren't using RTDs
        //XB.println(mc_temperatures_3.get_rtd_4_temperature());
        //XB.print("RTD 5 TEMP: ");
        //XB.println(mc_temperatures_3.get_rtd_5_temperature());
        XB.print("MOTOR TEMP: ");
        XB.println(mc_temperatures_3.get_motor_temperature() / (double) 10, 1);
        XB.print("TORQUE SHUDDER: ");
        XB.println(mc_temperatures_3.get_torque_shudder() / (double) 10, 1);*/
    }

    if (timer_debug_rms_motor_position_information.check()) {
        mc_motor_position_information.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_motor_position_information_t);
        xb_msg.id = ID_MC_MOTOR_POSITION_INFORMATION;
        write_xbee_data();
        /*XB.print("MOTOR ANGLE: ");
        XB.println(mc_motor_position_information.get_motor_angle());
        XB.print("MOTOR SPEED: ");
        XB.println(mc_motor_position_information.get_motor_speed());
        XB.print("ELEC OUTPUT FREQ: ");
        XB.println(mc_motor_position_information.get_electrical_output_frequency());
        XB.print("DELTA RESOLVER FILT: ");
        XB.println(mc_motor_position_information.get_delta_resolver_filtered());*/
    }

    if (timer_debug_rms_current_information.check()) {
        mc_current_information.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_current_information_t);
        xb_msg.id = ID_MC_CURRENT_INFORMATION;
        write_xbee_data();
        /*XB.print("PHASE A CURRENT: ");
        XB.println(mc_current_information.get_phase_a_current() / (double) 10, 1);
        XB.print("PHASE B CURRENT: ");
        XB.println(mc_current_information.get_phase_b_current() / (double) 10, 1);
        XB.print("PHASE C CURRENT: ");
        XB.println(mc_current_information.get_phase_c_current() / (double) 10, 1);
        XB.print("DC BUS CURRENT: ");
        XB.println(mc_current_information.get_dc_bus_current() / (double) 10, 1);*/
    }

    if (timer_debug_rms_voltage_information.check()) {
        mc_voltage_information.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_voltage_information_t);
        xb_msg.id = ID_MC_VOLTAGE_INFORMATION;
        write_xbee_data();
        /*XB.print("DC BUS VOLTAGE: ");
        XB.println(mc_voltage_information.get_dc_bus_voltage() / (double) 10, 1);
        XB.print("OUTPUT VOLTAGE: ");
        XB.println(mc_voltage_information.get_output_voltage() / (double) 10, 1);
        XB.print("PHASE AB VOLTAGE: ");
        XB.println(mc_voltage_information.get_phase_ab_voltage() / (double) 10, 1);
        XB.print("PHASE BC VOLTAGE: ");
        XB.println(mc_voltage_information.get_phase_bc_voltage() / (double) 10, 1);*/
    }

    if (timer_debug_rms_internal_states.check()) {
        mc_internal_states.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_internal_states_t);
        xb_msg.id = ID_MC_INTERNAL_STATES;
        write_xbee_data();
        /*XB.print("VSM STATE: ");
        XB.println(mc_internal_states.get_vsm_state());
        XB.print("INVERTER STATE: ");
        XB.println(mc_internal_states.get_inverter_state());
        XB.print("INVERTER RUN MODE: ");
        XB.println(mc_internal_states.get_inverter_run_mode());
        XB.print("INVERTER ACTIVE DISCHARGE STATE: ");
        XB.println(mc_internal_states.get_inverter_active_discharge_state());
        XB.print("INVERTER COMMAND MODE: ");
        XB.println(mc_internal_states.get_inverter_command_mode());
        XB.print("INVERTER ENABLE: ");
        XB.println(mc_internal_states.get_inverter_enable_state());
        XB.print("INVERTER LOCKOUT: ");
        XB.println(mc_internal_states.get_inverter_enable_lockout());
        XB.print("DIRECTION COMMAND: ");
        XB.println(mc_internal_states.get_direction_command());*/
    }

    if (timer_debug_rms_fault_codes.check()) {
        mc_fault_codes.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_fault_codes_t);
        xb_msg.id = ID_MC_FAULT_CODES;
        write_xbee_data();
        /*XB.print("POST FAULT LO: 0x");
        XB.println(mc_fault_codes.get_post_fault_lo(), HEX);
        XB.print("POST FAULT HI: 0x");
        XB.println(mc_fault_codes.get_post_fault_hi(), HEX);
        XB.print("RUN FAULT LO: 0x");
        XB.println(mc_fault_codes.get_run_fault_lo(), HEX);
        XB.print("RUN FAULT HI: 0x");
        XB.println(mc_fault_codes.get_run_fault_hi(), HEX);*/
    }

    if (timer_debug_rms_torque_timer_information.check()) {
        mc_torque_timer_information.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_torque_timer_information_t);
        xb_msg.id = ID_MC_TORQUE_TIMER_INFORMATION;
        write_xbee_data();
        /*XB.print("COMMANDED TORQUE: ");
        XB.println(mc_torque_timer_information.get_commanded_torque() / (double) 10, 1);
        XB.print("TORQUE FEEDBACK: ");
        XB.println(mc_torque_timer_information.get_torque_feedback());
        XB.print("RMS UPTIME: ");
        XB.println(mc_torque_timer_information.get_power_on_timer() * .003, 0);*/
    }

    if (timer_debug_bms_voltages.check()) {
        bms_voltages.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_bms_voltages_t);
        xb_msg.id = ID_BMS_VOLTAGES;
        write_xbee_data();
        /*XB.print("BMS VOLTAGE AVERAGE: ");
        XB.println(bms_voltages.get_average() / (double) 10000, 4);
        XB.print("BMS VOLTAGE LOW: ");
        XB.println(bms_voltages.get_low() / (double) 10000, 4);
        XB.print("BMS VOLTAGE HIGH: ");
        XB.println(bms_voltages.get_high() / (double) 10000, 4);
        XB.print("BMS VOLTAGE TOTAL: ");
        XB.println(bms_voltages.get_total() / (double) 100, 2);*/
    }

    if (timer_debug_bms_detailed_voltages.check()) {
        for (int ic = 0; ic < 8; ic++) {
            for (int group = 0; group < 3; group++) {
                bms_detailed_voltages[ic][group].write(xb_msg.buf);
                xb_msg.len = sizeof(CAN_message_bms_detailed_voltages_t);
                xb_msg.id = ID_BMS_DETAILED_VOLTAGES;
                write_xbee_data();
            }
        }
    }

    if (timer_debug_bms_temperatures.check()) {
        bms_temperatures.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_bms_temperatures_t);
        xb_msg.id = ID_BMS_TEMPERATURES;
        write_xbee_data();
        /*XB.print("BMS AVERAGE TEMPERATURE: ");
        XB.println(bms_temperatures.get_average_temperature() / (double) 100, 2);
        XB.print("BMS LOW TEMPERATURE: ");
        XB.println(bms_temperatures.get_low_temperature() / (double) 100, 2);
        XB.print("BMS HIGH TEMPERATURE: ");
        XB.println(bms_temperatures.get_high_temperature() / (double) 100, 2);*/
    }

    if (timer_debug_bms_detailed_temperatures.check()) {
        for (int ic = 0; ic < 8; ic++) {
            bms_detailed_temperatures[ic].write(xb_msg.buf);
            xb_msg.len = sizeof(CAN_message_bms_detailed_temperatures_t);
            xb_msg.id = ID_BMS_DETAILED_TEMPERATURES;
            write_xbee_data();
        }
    }

    if (timer_debug_bms_status.check()) {
        bms_status.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_bms_status_t);
        xb_msg.id = ID_BMS_STATUS;
        write_xbee_data();
        /*XB.print("BMS STATE: ");
        XB.println(bms_status.get_state());
        XB.print("BMS ERROR FLAGS: 0x");
        XB.println(bms_status.get_error_flags(), HEX);
        XB.print("BMS CURRENT: ");
        XB.println(bms_status.get_current() / (double) 100, 2);*/
    }

    if (timer_debug_rms_command_message.check()) {
        mc_command_message.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mc_command_message_t);
        xb_msg.id = ID_MC_COMMAND_MESSAGE;
        write_xbee_data();
        /*XB.print("CMD_MSG TORQUE COMMAND: ");
        XB.println(mc_command_message.get_torque_command() / (double) 10, 1);
        XB.print("CMD_MSG ANGULAR VELOCITY: ");
        XB.println(mc_command_message.get_angular_velocity());
        XB.print("CMD_MSG DIRECTION: ");
        XB.println(mc_command_message.get_direction());
        XB.print("CMD_MSG INVERTER ENABLE: ");
        XB.println(mc_command_message.get_inverter_enable());
        XB.print("CMD_MSG DISCHARGE ENABLE: ");
        XB.println(mc_command_message.get_discharge_enable());
        XB.print("CMD_MSG COMMANDED TORQUE LIMIT: ");
        XB.println(mc_command_message.get_commanded_torque_limit() / (double) 10, 1);*/
    }
    if (timer_debug_mcu_status.check()) {
        mcu_status.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mcu_status_t);
        xb_msg.id = ID_MCU_STATUS;
        write_xbee_data();
    }

    if (timer_debug_mcu_pedal_readings.check()) {
        mcu_pedal_readings.write(xb_msg.buf);
        xb_msg.len = sizeof(CAN_message_mcu_pedal_readings_t);
        xb_msg.id = ID_MCU_PEDAL_READINGS;
        write_xbee_data();
    }
    
    if (timer_debug_bms_balancing_status.check()) {
        for (int i = 0; i < 2; i++) {
            bms_balancing_status[i].write(xb_msg.buf);
            xb_msg.len = sizeof(CAN_message_bms_balancing_status_t);
            xb_msg.id = ID_BMS_BALANCING_STATUS;
            write_xbee_data();
        }
    }
}
