#include <SD.h>
#include <ADC_SPI.h>
#include <Wire.h>
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <Metro.h>

FlexCAN CAN(500000);
static CAN_message_t msg_rx;
static CAN_message_t msg_log;
File logger;

MCU_status mcu_status;
MCU_pedal_readings mcu_pedal_readings;
Current_readings current_readings;
BMS_voltages bms_voltages;
BMS_detailed_voltages bms_detailed_voltages;
BMS_temperatures bms_temperatures;
BMS_detailed_temperatures bms_detailed_temperatures;
BMS_onboard_temperatures bms_onboard_temperatures;
BMS_onboard_detailed_temperatures bms_onboard_detailed_temperatures;
BMS_status bms_status;
//BMS_balancing_status bms_balancing_status;                                                // class not yet implemented
CCU_status ccu_status;
MC_temperatures_1 mc_temperatures_1;
MC_temperatures_2 mc_temperatures_2;
MC_temperatures_3 mc_temperatures_3;
MC_analog_input_voltages mc_analog_input_voltages;
MC_digital_input_status mc_digital_input_status;
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
//static int flag_bms_balancing_status;                                                // class not yet implemented
static int flag_ccu_status;
static int flag_mc_temperatures_1;
static int flag_mc_temperatures_2;
static int flag_mc_temperatures_3;
static int flag_mc_analog_input_voltages;
static int flag_mc_digital_input_status;
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

void setup() {
  
  NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);                                   // Enables interrupts on the teensy for CAN messages
  attachInterruptVector(IRQ_CAN_MESSAGE, parse_can_message);          // Attaches parse_can_message() as ISR
  CAN0_IMASK1 |= 0x00000020;                                          // Allows "CAN message arrived" bit in flag register to throw interrupt
  
  // put your setup code here, to run once:
  pinMode(10, OUTPUT);                                                // Initialize pin 10 as output; this is necessary for the SD Library
  Serial.begin(115200);
  CAN.begin();    
  SD.begin(10);                                                       // Begin Arduino SD API (3.2)
  //SD.begin(BUILTIN_SDCARD);                                         // Begin Arduino SD API (3.5)
  logger = SD.open("sample.txt", FILE_WRITE);                         // Open file for writing.  
  logger.println("time, msg.id, data");                               // Print heading to the file.
  logger.flush();
}

void loop() {
  
  // check flags and write to SD therefrom
  if (flag_mcu_status) {
    mcu_status.write(msg_log.buf);
    msg_log.id = ID_MCU_STATUS;
    write_to_SD(flag_mcu_status);
  }
  if (flag_mcu_pedal_readings) {
    mcu_pedal_readings.write(msg_log.buf);
    msg_log.id = ID_MCU_PEDAL_READINGS;
    write_to_SD(flag_mcu_pedal_readings);
  }
  
  /*
   * . . . The rest to be written
   */
   
}

void parse_can_message() {

  // identify received CAN messages and load contents into corresponding structs
  while (CAN.read(msg_rx)) {
    int time_now = millis();                                          // to be replaced with RTC timeGet function
    if (msg_rx.id == ID_MCU_STATUS) {
      mcu_status.load(msg_rx.buf);
      flag_mcu_status = time_now;
    }
    if (msg_rx.id == ID_MCU_PEDAL_READINGS) {
      mcu_pedal_readings.load(msg_rx.buf);
      flag_mcu_status = time_now;
    }

    /*
     * . . . The rest to be written
     */
     
  }
}

void write_to_SD(int timestamp) {
  logger.print(timestamp);
  logger.print(",");
  logger.print(msg_log.id, HEX);
  logger.print(",");
  for (int i=0; i<msg_log.len; i++) {
    if (msg_log.buf[i]<16) {
      logger.print("0");
    }
    logger.print(msg_log.buf[i], HEX);
    logger.print(" ");
  }
  logger.println();
  logger.flush(); 
}

