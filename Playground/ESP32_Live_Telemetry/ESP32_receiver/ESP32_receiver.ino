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

  // Second Column
  values("requested_torque", "N/A");
  values("angular_velocity", "N/A");
  values("direction", "N/A");
  values("inverter_enable", "N/A");
  values("discharge_enable", "N/A");
  values("command_torque_limit", "N/A");
  values("mcu_state", "N/A");
  values("max_torque", "N/A");
  values("distance_travelled", "N/A");

  // BMS
  // On Board
  values("average_temperature", "N/A");
  values("low_temperature", "N/A");
  values("high_temperature", "N/A");
  // Actual BMS
  values("BMS_voltage_average", "N/A");
  values("BMS_voltage_low", "N/A");
  values("BMS_voltage_high", "N/A");
  values("BMS_voltage_total", "N/A");
  values("BMS_average_temperature", "N/A");
    
  // Sensors
  values("accelerator_pedal_1", "N/A");
  values("accelerator_pedal_2", "N/A");
  values("brake_transducer_1", "N/A");
  values("brake_transducer_2", "N/A");
  values("rpm_front_left", "N/A");
  values("rpm_front_right", "N/A");
  values("rpm_back_left", "N/A");
  values("rpm_back_right", "N/A");
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
  
  while (true) {
    Serial.println("##HYTECH RACING 2022 VEHICLE SERIAL DEBUGGER (ESP32)########################################################################################################################################");
    Serial.println("#");
    Serial.println("# CONSOLE STATUS: RUNNING");
    Serial.println("#");
    Serial.println("#");
    
    Serial.println("# RMS INVERTER\t\t\t\t\tBATTERY MANAGEMENT SYSTEM\t\t\t\tBATTERY MANAGEMENT SYSTEM DETAILED VOLTAGES");
    Serial.println("# ------------\t\t\t\t\t-------------------------\t\t\t\t-------------------------------------------");
    
    Serial.println("# MODULE A TEMPERATURE:\t\t   " + values["module_a_temperature"] + " ");
    Serial.println("# MODULE B TEMPERATURE:\t\t   " + values["module_b_temperature"] + " ");
    Serial.println("# MODULE C TEMPERATURE:\t\t   " + values["module_c_temperature"] + " ");
    Serial.println("# GATE DRIVER BOARD TEMPERATURE:   " + values["gate_driver_board_temperature"] + " ");
    Serial.println("# CONTROL BOARD TEMPERATURE:\t   " + values["control_board_temperature"] + " ");
    Serial.println("# RTD 1 TEMPERATURE:\t\t   " + values["rtd_1_temperature"] + " ");
    Serial.println("# RTD 2 TEMPERATURE:\t\t   " + values["rtd_2_temperature"] + " ");
    Serial.println("# RTD 3 TEMPERATURE:\t\t   " + values["rtd_3_temperature"] + " ");
    Serial.println("# RTD 4 TEMPERATURE:\t\t   " + values["rtd_4_temperature"] + " ");
    Serial.println("# RTD 5 TEMPERATURE:\t\t   " + values["rtd_5_temperature"] + " ");
    Serial.println("# MOTOR TEMPERATURE:\t\t   " + values["motor_temperature"] + " ");
    Serial.println("# TORQUE SHUDDER:\t\t   " + values["torque_shudder"] + " ");
    Serial.println("# MOTOR ANGLE:\t\t\t   " + values["motor_angle"] + " ");
    Serial.println("# MOTOR SPEED:\t\t\t   " + values["motor_speed"] + " ");
    Serial.println("# ELEC OUTPUT FREQ:\t\t   " + values["elec_output_freq"] + " ");
    Serial.println("# DELTA RESOLVER FILTERED:\t   " + values["delta_resolver_filtered"] + " ");
    Serial.println("# PHASE A CURRENT:\t\t   " + values["phase_a_current"] + " ");
    Serial.println("# PHASE B CURRENT:\t\t   " + values["phase_b_current"] + " ");
    Serial.println("# PHASE C CURRENT:\t\t   " + values["phase_c_current"] + " ");
    Serial.println("# DC BUS CURRENT:\t\t   " + values["dc_bus_current"] + " ");
    Serial.println("# DC BUS VOLTAGE:\t\t   " + values["dc_bus_voltage"] + " ");
    Serial.println("# OUTPUT VOLTAGE:\t\t   " + values["output_voltage"] + " ");
    Serial.println("# PHASE AB VOLTAGE:\t\t   " + values["phase_ab_voltage"] + " ");
    Serial.println("# PHASE BC VOLTAGE:\t\t   " + values["phase_bc_voltage"] + " ");
    Serial.println("# VSM STATE:\t\t\t   " + values["vsm_state"] + " ");
    Serial.println("# INVERTER STATE:\t\t   " + values["inverter_state"] + " ");
    Serial.println("# INVERTER RUN MODE:\t\t   " + values["inverter_run_mode"] + " ");
    Serial.println("# INVERTER ACTIVE DISCHARGE STATE: " + values["inverter_active_discharge_state"] + " ");
    Serial.println("# INVERTER COMMAND MODE:\t   " + values["inverter_command_mode"] + " ");
    Serial.println("# INVERTER ENABLE STATE:\t   " + values["inverter_enable_state"] + " ");
    Serial.println("# INVERTER ENABLE LOCKOUT:\t   " + values["inverter_enable_lockout"] + " ");
    Serial.println("# DIRECTION COMMAND:\t\t   " + values["direction_command"] + " ");
    Serial.println("# POST FAULT LO:\t\t   " + values["post_fault_lo"] + " ");
    Serial.println("# POST FAULT HI:\t\t   " + values["post_fault_hi"] + " ");
    Serial.println("# RUN FAULT LO:\t\t\t   " + values["run_fault_lo"] + " ");
    Serial.println("# RUN FAULT HI:\t\t\t   " + values["run_fault_hi"] + " ");
    Serial.println("# COMMANDED TORQUE:\t\t   " + values["commanded_torque"] + " ");
    Serial.println("# TORQUE FEEDBACK:\t\t   " + values["torque_feedback"] + " ");
    Serial.println("# RMS UPTIME:\t\t\t   " + values["rms_uptime"] + " ");
    Serial.println("#");
    Serial.println("#");
    Serial.println("# GLV CURRENT READINGS");
    Serial.println("# --------------------");
    Serial.println("# ECU CURRENT:\t\t\t   " + values["ecu_current"] + " ");
    Serial.println("# COOLING CURRENT:\t\t   " + values["cooling_current"] + " ");
    Serial.println("# TEMPERATURE:\t\t\t   " + values["temperature"] + " ");
    Serial.println("# GLV BATTERY VOLTAGE:\t\t   " + values["glv_battery_voltage"] + " ");
    Serial.println("#");
    Serial.println("#");
    Serial.println("#");
    Serial.println("#");
    Serial.println("#");
    Serial.println("#");
    Serial.println("#");
    Serial.println("#");
    Serial.println("############################################################################################################################################################################################");

    vTaskDelay(500 / portTICK_PERIOD_MS);
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
    3, // Highest Priority Task
    NULL,
    app_cpu);

  xTaskCreatePinnedToCore(
    print_to_serial,
    "Print to Serial",
    8192,
    NULL,
    1,
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
 * @brief: Clear terminal with VT100 ANSI codes (cannot work with Arduino Serial Terminal, use Teraterm, etc.)
 */
void clear_screen(void) {
  char clear_cmd[]= {0x1b,'[','1','J',0};
  char home_cursor_cmd[] = {0x1b , '[', '1', ';', '1', 'H', 0};
  Serial.print(clear_cmd);
  Serial.print(home_cursor_cmd);
}
