/**
 * @brief:
 * 
 * @author:
 * @date:
 */
 
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Variables and Imports
/////////////////////////////////////////////////////////////////////////////////////////////////////

// FreeRTOS
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define QUEUE_LEN 255
#define MOCK_MSG true // Fake task to spawn raw messages to test module, set to false in the field
static QueueHandle_t raw_msg_queue;
static QueueHandle_t parsed_msg_queue;

// ESP-Now
#include <esp_now.h>
#include <WiFi.h>

typedef struct transmit_msg_t {
  char* label;
  char* raw_data;
} transmit_msg_t;

static transmit_msg_t incoming_msg;
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len);

// Parsing and Serial Monitor
#include <Dictionary.h>

Dictionary &values = *(new Dictionary());
Dictionary &units = *(new Dictionary());

void clear_screen(void);

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Task Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief: If MOCK_MSG is false, receives messages transmitted via ESP-Now and adds to the raw message queue.
 *         If MOCK_MSG is true, randomly spawns a raw message every 100 ms and adds to the raw message queue.
 */
void add_incoming_msg(void * no_params) {
  #if MOCK_MSG
  // Randomly spawns a message every 100 ms for testing
  while (true) {
    incoming_msg.label = "A5";
    incoming_msg.raw_data = "770B000000009002";
    xQueueSend(raw_msg_queue, (void *) &incoming_msg, 100);

    vTaskDelay(100 / portTICK_PERIOD_MS); // delay for 100 ms
  }
  
  #else
  // Actual ESP-Now connecion
  WiFi.mode(WIFI_STA); // Set ESP32 to stationary mode

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    exit(0); // Terminate program if cannot establish ESP-Now comms and wait for reset
  }

  esp_now_register_recv_cb(data_receive); // Register callback function
  #endif
}

/**
 * @brief: Handles all serial monitor transactions
 */
void print_to_serial(void * no_params) {
  transmit_msg_t item;

  // Initialize dictionary items
  // First Column
  values("module_a_temperature", "N/A");
  values("module_a_temperature", "N/A");
  values("module_b_temperature", "N/A");
  values("module_c_temperature", "N/A");
  values("gate_driver_board_temperature", "N/A");
  values("control_board_temperature", "N/A");
  values("rtd_1_temperature", "N/A");
  values("rtd_2_temperature", "N/A");
  values("rtd_3_temperature", "N/A");
  values("rtd_4_temperature", "N/A");
  values("rtd_5_temperature", "N/A");
  values("motor_temperature", "N/A");
  values("torque_shudder", "N/A");
  values("motor_angle", "N/A");
  values("motor_speed", "N/A");
  values("elec_output_freq", "N/A");
  values("delta_resolver_filtered", "N/A");
  values("phase_a_current", "N/A");
  values("phase_b_current", "N/A");
  values("phase_c_current", "N/A");
  values("dc_bus_current", "N/A");
  values("dc_bus_voltage", "N/A");
  values("output_voltage", "N/A");
  values("phase_ab_voltage", "N/A");
  values("phase_bc_voltage", "N/A");
  values("vsm_state", "N/A");
  values("inverter_state", "N/A");
  values("inverter_run_mode", "N/A");
  values("inverter_active_discharge_state", "N/A");
  values("inverter_command_mode", "N/A");
  values("inverter_enable_state", "N/A");
  values("inverter_enable_lockout", "N/A");
  values("direction_command", "N/A");
  values("post_fault_lo", "N/A");
  values("post_fault_hi", "N/A");
  values("run_fault_lo", "N/A");
  values("run_fault_hi", "N/A");
  values("commanded_torque", "N/A");
  values("torque_feedback", "N/A");
  values("rms_uptime", "N/A");
  values("ecu_current", "N/A");
  values("cooling_current", "N/A");
  values("temperature", "N/A");
  values("glv_battery_voltage", "N/A");
  values("rpm_front_left", "N/A");
  values("rpm_front_right", "N/A");
  values("rpm_back_left", "N/A");
  values("rpm_back_right", "N/A");
  values("distance_travelled", "N/A");

  units("module_a_temperature", "");
  units("module_a_temperature", "");
  units("module_b_temperature", "");
  units("module_c_temperature", "");
  units("gate_driver_board_temperature", "");
  units("control_board_temperature", "");
  units("rtd_1_temperature", "");
  units("rtd_2_temperature", "");
  units("rtd_3_temperature", "");
  units("rtd_4_temperature", "");
  units("rtd_5_temperature", "");
  units("motor_temperature", "");
  units("torque_shudder", "");
  units("motor_angle", "");
  units("motor_speed", "");
  units("elec_output_freq", "");
  units("delta_resolver_filtered", "");
  units("phase_a_current", "");
  units("phase_b_current", "");
  units("phase_c_current", "");
  units("dc_bus_current", "");
  units("dc_bus_voltage", "");
  units("output_voltage", "");
  units("phase_ab_voltage", "");
  units("phase_bc_voltage", "");
  units("vsm_state", "");
  units("inverter_state", "");
  units("inverter_run_mode", "");
  units("inverter_active_discharge_state", "");
  units("inverter_command_mode", "");
  units("inverter_enable_state", "");
  units("inverter_enable_lockout", "");
  units("direction_command", "");
  units("post_fault_lo", "");
  units("post_fault_hi", "");
  units("run_fault_lo", "");
  units("run_fault_hi", "");
  units("commanded_torque", "");
  units("torque_feedback", "");
  units("rms_uptime", "");
  units("ecu_current", "");
  units("cooling_current", "");
  units("temperature", "");
  units("glv_battery_voltage", "");
  units("rpm_front_left", "");
  units("rpm_front_right", "");
  units("rpm_back_left", "");
  units("rpm_back_right", "");
  units("distance_travelled", "");

  // Second Column
  values("BMS_average_temperature", "N/A");
  values("BMS_low_temperature", "N/A");
  values("BMS_high_temperature", "N/A");
  values("BMS_state", "N/A");
  values("BMS_error_flags", "N/A");
  values("BMS_current", "N/A");
  values("BMS_voltage_average", "N/A");
  values("BMS_voltage_low", "N/A");
  values("BMS_voltage_high", "N/A");
  values("BMS_voltage_total", "N/A");
  values("BMS_total_charge", "N/A");
  values("BMS_total_discharge", "N/A");
  values("imd_ok_high", "N/A");
  values("bms_ok_high", "N/A");
  values("bspd_ok_high", "N/A");
  values("software_ok_high", "N/A");
  values("shutdown_b_above_threshold", "N/A");
  values("shutdown_c_above_threshold", "N/A");
  values("shutdown_d_above_threshold", "N/A");
  values("shutdown_e_above_threshold", "N/A");
  values("inverter_powered", "N/A");
  values("accelerator_pedal_1", "N/A");
  values("accelerator_pedal_2", "N/A");
  values("brake_transducer_1", "N/A");
  values("brake_transducer_2", "N/A");
  values("no_accel_implausability", "N/A");
  values("no_brake_implausability", "N/A");
  values("brake_pedal_active", "N/A");
  values("mcu_state", "N/A");
  values("torque_mode", "N/A");
  values("max_torque", "N/A");
  values("requested_torque", "N/A");
  values("fl_susp_lin_pot", "N/A");
  values("fr_susp_lin_pot", "N/A");
  values("bl_susp_lin_pot", "N/A");
  values("br_susp_lin_pot", "N/A");
  values("amb_air_temp", "N/A");
  values("amb_air_hum", "N/A");
  values("current", "N/A");
  values("current_gain", "N/A");
  values("voltage", "N/A");
  values("voltage_gain", "N/A");
  values("lat_accel", "N/A");
  values("long_accel", "N/A");
  values("vert_accel", "N/A");
  values("yaw", "N/A");
  values("pitch", "N/A");
  values("roll", "N/A");

  units("BMS_average_temperature", "");
  units("BMS_low_temperature", "");
  units("BMS_high_temperature", "");
  units("BMS_state", "");
  units("BMS_error_flags", "");
  units("BMS_current", "");
  units("BMS_voltage_average", "");
  units("BMS_voltage_low", "");
  units("BMS_voltage_high", "");
  units("BMS_voltage_total", "");
  units("BMS_total_charge", "");
  units("BMS_total_discharge", "");
  units("imd_ok_high", "");
  units("bms_ok_high", "");
  units("bspd_ok_high", "");
  units("software_ok_high", "");
  units("shutdown_b_above_threshold", "");
  units("shutdown_c_above_threshold", "");
  units("shutdown_d_above_threshold", "");
  units("shutdown_e_above_threshold", "");
  units("inverter_powered", "");
  units("accelerator_pedal_1", "");
  units("accelerator_pedal_2", "");
  units("brake_transducer_1", "");
  units("brake_transducer_2", "");
  units("no_accel_implausability", "");
  units("no_brake_implausability", "");
  units("brake_pedal_active", "");
  units("mcu_state", "");
  units("torque_mode", "");
  units("max_torque", "");
  units("requested_torque", "");
  units("fl_susp_lin_pot", "");
  units("fr_susp_lin_pot", "");
  units("bl_susp_lin_pot", "");
  units("br_susp_lin_pot", "");
  units("amb_air_temp", "");
  units("amb_air_hum", "");
  units("current", "");
  units("current_gain", "");
  units("voltage", "");
  units("voltage_gain", "");
  units("lat_accel", "");
  units("long_accel", "");
  units("vert_accel", "");
  units("yaw", "");
  units("pitch", "");
  units("roll", "");

  // Third Column
  values("current", "N/A");
  values("voltage", "N/A");
  values("current_gain", "N/A");
  values("voltage_gain", "N/A");
  
  units("current", "");
  units("voltage", "");
  units("current_gain", "");
  units("voltage_gain", "");
  
  while (true) {
    Serial.println("##HYTECH RACING 2022 VEHICLE SERIAL DEBUGGER (ESP32)########################################################################################################################################");
    Serial.println("#");
    Serial.println("# CONSOLE STATUS: RUNNING");
    Serial.println("#");
    
    Serial.println("# RMS INVERTER\t\t\t\t\t   BATTERY MANAGEMENT SYSTEM\t\t\t\t\tBATTERY MANAGEMENT SYSTEM DETAILED VOLTAGES");
    Serial.println("# ------------\t\t\t\t\t   -------------------------\t\t\t\t\t-------------------------------------------");
    
    Serial.println("# MODULE A TEMPERATURE:\t\t   "       + (values["module_a_temperature"] + " " + units["module_a_temperature"] + "            ").substring(0, 20)                          + "BMS AVERAGE TEMPERATURE: ");
    Serial.println("# MODULE B TEMPERATURE:\t\t   "       + (values["module_b_temperature"] + " " + units["module_b_temperature"] + "            ").substring(0, 20)                          + "BMS LOW TEMPERATURE: ");
    Serial.println("# MODULE C TEMPERATURE:\t\t   "       + (values["module_c_temperature"] + " " + units["module_c_temperature"] + "            ").substring(0, 20)                          + "BMS HIGH TEMPERATURE: ");
    Serial.println("# GATE DRIVER BOARD TEMPERATURE:   "  + (values["gate_driver_board_temperature"] + " " + units["gate_driver_board_temperature"] + "            ").substring(0, 20)        + "BMS STATE: ");
    Serial.println("# CONTROL BOARD TEMPERATURE:\t   "    + (values["control_board_temperature"] + " " + units["control_board_temperature"] + "            ").substring(0, 20)                + "BMS ERROR FLAGS: ");
    Serial.println("# RTD 1 TEMPERATURE:\t\t   "          + (values["rtd_1_temperature"] + " " + units["rtd_1_temperature"] + "            ").substring(0, 20)                                + "BMS CURRENT: ");
    Serial.println("# RTD 2 TEMPERATURE:\t\t   "          + (values["rtd_2_temperature"] + " " + units["rtd_2_temperature"] + "            ").substring(0, 20)                                + "BMS VOLTAGE AVERAGE: ");
    Serial.println("# RTD 3 TEMPERATURE:\t\t   "          + (values["rtd_3_temperature"] + " " + units["rtd_3_temperature"] + "            ").substring(0, 20)                                + "BMS VOLTAGE LOW: ");
    Serial.println("# RTD 4 TEMPERATURE:\t\t   "          + (values["rtd_4_temperature"] + " " + units["rtd_4_temperature"] + "            ").substring(0, 20)                                + "BMS VOLTAGE HIGH: ");
    Serial.println("# RTD 5 TEMPERATURE:\t\t   "          + (values["rtd_5_temperature"] + " " + units["rtd_5_temperature"] + "            ").substring(0, 20)                                + "BMS VOLTAGE TOTAL: ");
    Serial.println("# MOTOR TEMPERATURE:\t\t   "          + (values["motor_temperature"] + " " + units["motor_temperature"] + "            ").substring(0, 20)                                + "BMS TOTAL CHARGE: ");
    Serial.println("# TORQUE SHUDDER:\t\t   "             + (values["torque_shudder"] + " " + units["torque_shudder"] + "            ").substring(0, 20)                                      + "BMS TOTAL DISCHARGE: ");
    Serial.println("# MOTOR ANGLE:\t\t\t   "              + (values["motor_angle"] + " " + units["motor_angle"] + "            ").substring(0, 20)                                            + "");
    Serial.println("# MOTOR SPEED:\t\t\t   "              + (values["motor_speed"] + " " + units["motor_speed"] + "            ").substring(0, 20)                                            + "MAIN CONTROL UNIT");
    Serial.println("# ELEC OUTPUT FREQ:\t\t   "           + (values["elec_output_freq"] + " " + units["elec_output_freq"] + "            ").substring(0, 20)                                  + "-----------------");
    Serial.println("# DELTA RESOLVER FILTERED:\t   "      + (values["delta_resolver_filtered"] + " " + units["delta_resolver_filtered"] + "            ").substring(0, 20)                    + "IMD OK HIGH: ");
    Serial.println("# PHASE A CURRENT:\t\t   "            + (values["phase_a_current"] + " " + units["phase_a_current"] + "            ").substring(0, 20)                                    + "BMS OK HIGH: ");
    Serial.println("# PHASE B CURRENT:\t\t   "            + (values["phase_b_current"] + " " + units["phase_b_current"] + "            ").substring(0, 20)                                    + "BSPD OK HIGH: ");
    Serial.println("# PHASE C CURRENT:\t\t   "            + (values["phase_c_current"] + " " + units["phase_c_current"] + "            ").substring(0, 20)                                    + "SOFTWARE OK HIGH: ");
    Serial.println("# DC BUS CURRENT:\t\t   "             + (values["dc_bus_current"] + " " + units["dc_bus_current"] + "            ").substring(0, 20)                                      + "SHUTDOWN B ABOVE THRESHOLD: ");
    Serial.println("# DC BUS VOLTAGE:\t\t   "             + (values["dc_bus_voltage"] + " " + units["dc_bus_voltage"] + "            ").substring(0, 20)                                      + "SHUTDOWN C ABOVE THRESHOLD: ");
    Serial.println("# OUTPUT VOLTAGE:\t\t   "             + (values["output_voltage"] + " " + units["output_voltage"] + "            ").substring(0, 20)                                      + "SHUTDOWN D ABOVE THRESHOLD: ");
    Serial.println("# PHASE AB VOLTAGE:\t\t   "           + (values["phase_ab_voltage"] + " " + units["phase_ab_voltage"] + "            ").substring(0, 20)                                  + "SHUTDOWN E ABOVE THRESHOLD: ");
    Serial.println("# PHASE BC VOLTAGE:\t\t   "           + (values["phase_bc_voltage"] + " " + units["phase_bc_voltage"] + "            ").substring(0, 20)                                  + "INVERTER POWERED: ");
    Serial.println("# VSM STATE:\t\t\t   "                + (values["vsm_state"] + " " + units["vsm_state"] + "            ").substring(0, 20)                                                + "ACCELERATOR PEDAL 1: ");
    Serial.println("# INVERTER STATE:\t\t   "             + (values["inverter_state"] + " " + units["inverter_state"] + "            ").substring(0, 20)                                      + "ACCELERATOR PEDAL 2: ");
    Serial.println("# INVERTER RUN MODE:\t\t   "          + (values["inverter_run_mode"] + " " + units["inverter_run_mode"] + "            ").substring(0, 20)                                + "BRAKE TRANSDUCER 1: ");
    Serial.println("# INVERTER ACTIVE DISCHARGE STATE: "  + (values["inverter_active_discharge_state"] + " " + units["inverter_active_discharge_state"] + "            ").substring(0, 20)    + "BRAKE TRANSDUCER 2: ");
    Serial.println("# INVERTER COMMAND MODE:\t   "        + (values["inverter_command_mode"] + " " + units["inverter_command_mode"] + "            ").substring(0, 20)                        + "NO ACCEL IMPLAUSIBILITY: ");
    Serial.println("# INVERTER ENABLE STATE:\t   "        + (values["inverter_enable_state"] + " " + units["inverter_enable_state"] + "            ").substring(0, 20)                        + "NO BRAKE IMPLAUSIBILITY: ");
    Serial.println("# INVERTER ENABLE LOCKOUT:\t   "      + (values["inverter_enable_lockout"] + " " + units["inverter_enable_lockout"] + "            ").substring(0, 20)                    + "BRAKE PEDAL ACTIVE: ");
    Serial.println("# DIRECTION COMMAND:\t\t   "          + (values["direction_command"] + " " + units["direction_command"] + "            ").substring(0, 20)                                + "MCU STATE: ");
    Serial.println("# POST FAULT LO:\t\t   "              + (values["post_fault_lo"] + " " + units["post_fault_lo"] + "            ").substring(0, 20)                                        + "TORQUE MODE: ");
    Serial.println("# POST FAULT HI:\t\t   "              + (values["post_fault_hi"] + " " + units["post_fault_hi"] + "            ").substring(0, 20)                                        + "MAX TORQUE: ");
    Serial.println("# RUN FAULT LO:\t\t\t   "             + (values["run_fault_lo"] + " " + units["run_fault_lo"] + "            ").substring(0, 20)                                          + "REQUESTED TORQUE: ");
    Serial.println("# RUN FAULT HI:\t\t\t   "             + (values["run_fault_hi"] + " " + units["run_fault_hi"] + "            ").substring(0, 20)                                          + "");
    Serial.println("# COMMANDED TORQUE:\t\t   "           + (values["commanded_torque"] + " " + units["commanded_torque"] + "            ").substring(0, 20)                                  + "SENSOR ACQUISITION BOARD");
    Serial.println("# TORQUE FEEDBACK:\t\t   "            + (values["torque_feedback"] + " " + units["torque_feedback"] + "            ").substring(0, 20)                                    + "------------------------");
    Serial.println("# RMS UPTIME:\t\t\t   "               + (values["rms_uptime"] + " " + units["rms_uptime"] + "            ").substring(0, 20)                                              + "FRONT LEFT POTENTIOMETER: ");
    Serial.println(String("#\t\t\t\t\t\t   ")                                                                                                                                                 + "FRONT RIGHT POTENTIOMETER: ");
    Serial.println(String("# GLV CURRENT READINGS\t\t\t\t   ")                                                                                                                                + "BACK LEFT POTENTIOMETER: ");
    Serial.println(String("# --------------------\t\t\t\t   ")                                                                                                                                + "BACK RIGHT POTENTIOMETER: ");
    Serial.println("# ECU CURRENT:\t\t\t   "              + (values["ecu_current"] + " " + units["ecu_current"] + "            ").substring(0, 20)                                            + "AMBIENT AIR TEMPERATURE: ");
    Serial.println("# COOLING CURRENT:\t\t   "            + (values["cooling_current"] + " " + units["cooling_current"] + "            ").substring(0, 20)                                    + "AMBIENT AIR HUMIDITY: ");
    Serial.println("# TEMPERATURE:\t\t\t   "              + (values["temperature"] + " " + units["temperature"] + "            ").substring(0, 20)                                            + "LATERAL ACCELERATION: ");
    Serial.println("# GLV BATTERY VOLTAGE:\t\t   "        + (values["glv_battery_voltage"] + " " + units["glv_battery_voltage"] + "            ").substring(0, 20)                            + "LONGITUDINAL ACCELERATION: ");
    Serial.println(String("#\t\t\t\t\t\t   ")                                                                                                                                                 + "VERTICAL ACCELERATION: ");
    Serial.println(String("# WHEEL SPEED SENSORS\t\t\t\t   ")                                                                                                                                 + "YAW: ");
    Serial.println(String("# -------------------\t\t\t\t   ")                                                                                                                                 + "PITCH: ");
    Serial.println("# RPM FRONT LEFT:\t\t   "             + (values["rpm_front_left"] + " " + units["rpm_front_left"] + "            ").substring(0, 20)                                      + "ROLL: ");
    Serial.println("# RPM FRONT RIGHT\t\t   "             + (values["rpm_front_right"] + " " + units["rpm_front_right"] + "            ").substring(0, 20)                                    + "");
    Serial.println("# RPM BACK LEFT:\t\t   "              + (values["rpm_back_left"] + " " + units["rpm_back_left"] + "            ").substring(0, 20)                                        + "");
    Serial.println("# RPM BACK RIGHT:\t\t   "             + (values["rpm_back_right"] + " " + units["rpm_back_right"] + "            ").substring(0, 20)                                      + "");
    Serial.println(String("#\t\t\t\t\t\t   ")                                                                                                                                                 + "");
    Serial.println("# DISTANCE TRAVELLED:\t\t   "         + (values["distance_travelled"] + " " + units["distance_travelled"] + "            ").substring(0, 20)                              + "");
    Serial.println(String("#\t\t\t\t\t\t   ")                                                                                                                                                 + "");
    Serial.println("############################################################################################################################################################################################");

    vTaskDelay(100 / portTICK_PERIOD_MS);
    clear_screen();
  }
}

void setup() {
  // Initialize resources shared by multiple tasks
  Serial.begin(921600); // High baud-rate, will not work with Arduino Serial Terminal
  raw_msg_queue = xQueueCreate(QUEUE_LEN, sizeof(transmit_msg_t));

  // Create tasks
  xTaskCreatePinnedToCore(
    add_incoming_msg,
    "Add Incoming Message",
    2048,
    NULL,
    3,
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    print_to_serial,
    "Print to Serial",
    8192,
    NULL,
    10, // Highest Priority Task
    NULL,
    app_cpu);
  
  vTaskDelete(NULL); // Delete setup() and loop() task
}

void loop() {
  // Not needed in FreeRTOS Implementation
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief: Copies in the received raw message and adds it to the raw message queue
 * 
 * @param[in]: mac - pointer to the MAC address, automatically filled in and not used
 * @param[in]: incomingData - pointer to the data, automatically filled in
 * @param[in]: len - length of the data received, automatically filled in and not used
 */
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incoming_msg, incomingData, sizeof(transmit_msg_t)); // Copy in data
  xQueueSend(raw_msg_queue, (void *) &incoming_msg, 100);      // Add to queue, fail if timeout after 100 ms
}

/**
 * @brief: Clear terminal with VT100 ANSI codes (cannot work with Arduino Serial Terminal, use Teraterm, etc.) by moving the cursor to starting location
 */
void clear_screen(void) {
  char home_cursor_cmd[] = {0x1b , '[', '1', ';', '1', 'H', 0};
  Serial.print(home_cursor_cmd);
}
