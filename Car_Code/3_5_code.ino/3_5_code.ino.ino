#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <ADC_SPI.h>
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>
#include <Metro.h>
#include <XBTools.h>

#define XB Serial2

#define BMS_HIGH 134 // ~3V on BMS_OK line
#define IMD_HIGH 134 // ~3V on OKHS line
#define SHUTDOWN_OUT_HIGH 350 // ~5V on SHUTDOWN_C line
#define XBEE_PKT_LEN 15

FlexCAN CAN(500000);
static CAN_message_t msg;
static CAN_message_t xb_msg;
File logger;
FCU_accelerometer_values fcu_accelerometer_values;
ADC_SPI ADC(10);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

MC_command_message mc_command_message;
MC_temperatures_1 mc_temperatures_1;
MC_temperatures_3 mc_temperatures_3;
MC_motor_position_information mc_motor_position_information;
MC_current_information mc_current_information;
MC_voltage_information mc_voltage_information;
MC_internal_states mc_internal_states;
MC_fault_codes mc_fault_codes;
MC_torque_timer_information mc_torque_timer_information;
BMS_voltages bms_voltages;
BMS_detailed_voltages bms_detailed_voltages[8][3];
BMS_temperatures bms_temperatures;
BMS_detailed_temperatures bms_detailed_temperatures[8];
BMS_status bms_status;
Current_readings CAN_current_sensor_readings;


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

Metro timer_debug_RTC = Metro(1000);

/*
 * To correctly push this code onto the Teensy, you must push this code TWICE: first with the Teensy3Clock.set([time]) function below uncommented and the current epoch time inserted as the parameter [time].  This is to set the current time onto the RTC onboard.
 * Next, comment out the same line and push again.  This ensures that each time the Teensy "wakes", that same time parameter will not be set again. 
 */

void setup() {
  // put your setup code here, to run once:
    NVIC_ENABLE_IRQ(IRQ_CAN0_MESSAGE);                                   // Enables interrupts on the teensy for CAN messages
    attachInterruptVector(IRQ_CAN0_MESSAGE, parse_can_message);          // Attaches parse_can_message() as ISR
    FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;                             // Allows "CAN message arrived" bit in flag register to throw interrupt
    CAN0_MCR &= 0xFFFDFFFF;                                             // Enables CAN message self-reception

    Serial.begin(115200);
    CAN.begin();                                                        // Begin CAN
    SD.begin(BUILTIN_SDCARD);                                           // Begin Arduino SD API
    pinMode(10, OUTPUT);                                                // Initialize pin 10 as output; this is necessary for the SD Library
    logger = SD.open("sample.txt", FILE_WRITE);                         // Open file for writing.  Code revision necessary to record distinct sessions
    logger.println("time (uS), msg.id, data");                          // Print heading to the file.
    logger.flush();

    //Teensy3Clock.set(1536653782);                                       // set time (epoch) at powerup  (COMMENT OUT THIS LINE AND PUSH ONCE RTC HAS BEEN SET!!!!)
    setSyncProvider(getTeensy3Time);                                    // registers Teensy RTC as system time

    setupAccelerometer();

    pinMode(A12, INPUT);
    pinMode(A13, INPUT);

    XB.begin(115200);
}

void loop() {
    send_xbee();

    if (timer_debug_RTC.check()) {
      digitalClockDisplay();
    }
    
    if (timer_accelerometer.check()) {
        processAccelerometer(); 
    }
    if (timer_current.check()) {
      //self derived
      double current_ecu = ((double)(analogRead(A13)-96))*0.029412;
      double current_cooling = ((double)(analogRead(A12)-96))*0.029412;
      Serial.println(current_cooling);
      Serial.println(current_ecu);
      
      CAN_current_sensor_readings.set_ecu_current_value((short)((int)(current_ecu*100)));
      CAN_current_sensor_readings.set_cooling_current_value((short)((int)(current_cooling*100)));

      noInterrupts();
      CAN_current_sensor_readings.write(msg.buf);
      msg.id = ID_ECU_CURRENT_SENSOR_READINGS;
      msg.len = sizeof(CAN_current_sensor_readings_t);
      CAN.write(msg);

      CAN_current_sensor_readings.write(xb_msg.buf);
      xb_msg.id = ID_ECU_CURRENT_SENSOR_READINGS;
      xb_msg.len = sizeof(CAN_current_sensor_readings_t);
      write_xbee_data();
      interrupts();
     
    }
}

void parse_can_message() {                                              // ISR
   while (CAN.read(msg)) {
    
    Serial.println("Received!!");
    timestampWrite();
    logger.print(msg.id, HEX);
    logger.print(", ");
    for (int i=0; i<msg.len; i++) {
      logger.print(msg.buf[i], HEX);
      logger.print(" ");
    }
    logger.println();
    logger.flush();                                                       // Ensure log is saved
    
        
        if (msg.id == ID_MC_COMMAND_MESSAGE) {
            mc_command_message.load(msg.buf);
        }
        if (msg.id == ID_MC_TEMPERATURES_1) {
            mc_temperatures_1.load(msg.buf);
        }
        if (msg.id == ID_MC_TEMPERATURES_3) {
            mc_temperatures_3.load(msg.buf);
        }
        if (msg.id == ID_MC_MOTOR_POSITION_INFORMATION) {
            mc_motor_position_information.load(msg.buf);
        }
        if (msg.id == ID_MC_CURRENT_INFORMATION) {
            mc_current_information.load(msg.buf);
        }
        if (msg.id == ID_MC_VOLTAGE_INFORMATION) {
            mc_voltage_information.load(msg.buf);
        }
        if (msg.id == ID_MC_INTERNAL_STATES) {
            mc_internal_states.load(msg.buf);
        }
        if (msg.id == ID_MC_FAULT_CODES) {
            mc_fault_codes.load(msg.buf);
        }
        if (msg.id == ID_MC_TORQUE_TIMER_INFORMATION) {
            mc_torque_timer_information.load(msg.buf);
        }
        if (msg.id == ID_BMS_VOLTAGES) {
            bms_voltages.load(msg.buf);
        }
        if (msg.id == ID_BMS_TEMPERATURES) {
            bms_temperatures.load(msg.buf);
        }
        if (msg.id == ID_BMS_DETAILED_TEMPERATURES) {
            BMS_detailed_temperatures temp = BMS_detailed_temperatures(msg.buf);
            bms_detailed_temperatures[temp.get_ic_id()].load(msg.buf);
        }
        if (msg.id == ID_BMS_STATUS) {
            bms_status.load(msg.buf);
        }
        if (msg.id == ID_BMS_DETAILED_VOLTAGES) {
            BMS_detailed_voltages temp = BMS_detailed_voltages(msg.buf);
            bms_detailed_voltages[temp.get_ic_id()][temp.get_group_id()].load(msg.buf);
        }
    }
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void timestampWrite() {
  logger.print(hour());
  writeDigits(minute());
  writeDigits(second());
  logger.print(" ");
  logger.print(day());
  logger.print(" ");
  logger.print(month());
  logger.print(" ");
  logger.print(year()); 
  logger.print("   ");
}

/*
 * What the frick is this function used for????????
 */
time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void writeDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  logger.print(":");
  if(digits < 10)
    logger.print('0');
  logger.print(digits);
}

void setupAccelerometer() {
      
  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Sensor not detected!!!!!");
  }
  else {
    accel.setRange(ADXL345_RANGE_4_G);
  }
}

void processAccelerometer() {
  /* Get a new sensor event */ 
  sensors_event_t event; 
  accel.getEvent(&event);
  
  /* Read accelerometer values into accelerometer struct */
  fcu_accelerometer_values.set_values((short)((int)(event.acceleration.x*100)), (short)((int)(event.acceleration.y*100)), (short)((int)(event.acceleration.z*100)));
  
  /* Send msg over CAN */
  noInterrupts();
  fcu_accelerometer_values.write(xb_msg.buf);
  xb_msg.id = ID_FCU_ACCELEROMETER;
  xb_msg.len = sizeof(CAN_message_fcu_accelerometer_values_t);
  write_xbee_data();
  fcu_accelerometer_values.write(msg.buf);
  msg.id = ID_FCU_ACCELEROMETER;
  msg.len = sizeof(CAN_message_fcu_accelerometer_values_t);
  CAN.write(msg);
  interrupts();

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
}
