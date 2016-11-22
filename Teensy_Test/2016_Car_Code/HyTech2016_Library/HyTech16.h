#ifndef __HYTECH16_H__
#define __HYTECH16_H__

#include <Arduino.h>
#include <string.h>

/*
 * ECU state definitions
 */
#define PCU_STATE_WAITING_BMS_IMD 1
#define PCU_STATE_WAITING_DRIVER 2
#define PCU_STATE_LATCHING 3
#define PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED 4
#define PCU_STATE_FATAL_FAULT 5
#define TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED 1
#define TCU_STATE_TRACTIVE_SYSTEM_ACTIVE 2
#define TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE 3
#define TCU_STATE_ENABLING_INVERTER 4
#define TCU_STATE_WAITING_READY_TO_DRIVE_SOUND 5
#define TCU_STATE_READY_TO_DRIVE 6

/*
 * CAN ID definitions
 */
#define ID_PCU_STATUS 0xD0
#define ID_TCU_STATUS 0xD1
#define ID_MC_INTERNAL_STATES 0xAA
#define ID_MC_VOLTAGE_INFORMATION 0xA7
#define ID_MC_FAULT_CODES 0xAB

typedef struct CAN_message_pcu_status_t {
  uint8_t state;
  bool bms_fault;
  bool imd_fault;
} CAN_msg_pcu_status;

class PCU_status {
  public:
    PCU_status();
    PCU_status(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    void write(uint8_t buf[8]);
    uint8_t get_state();
    bool get_bms_fault();
    bool get_imd_fault();
    void set_state(uint8_t state);
    void set_bms_fault(bool bms_fault);
    void set_imd_fault(bool imd_fault);
  private:
    CAN_message_pcu_status_t message;
};

typedef struct CAN_message_tcu_status_t {
  uint8_t state;
  uint8_t btn_start_id;
} CAN_msg_tcu_status;

class TCU_status {
  public:
    TCU_status();
    TCU_status(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    void write(uint8_t buf[8]);
    uint8_t get_state();
    uint8_t get_btn_start_id();
    void set_state(uint8_t state);
    void set_btn_start_id(uint8_t btn_start_id);
  private:
    CAN_message_tcu_status_t message;
};

typedef struct CAN_message_mc_voltage_information_t {
  int16_t dc_bus_voltage;
  int16_t output_voltage;
  int16_t phase_ab_voltage;
  int16_t phase_bc_voltage;
} CAN_message_mc_voltage_information_t;

class MC_voltage_information {
  public:
    MC_voltage_information();
    MC_voltage_information(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    float get_dc_bus_voltage();
    float get_output_voltage();
    float get_phase_ab_voltage();
    float get_phase_bc_voltage();
  private:
    CAN_message_mc_voltage_information_t message;
};

typedef struct CAN_message_mc_internal_states_t {
  uint16_t vsm_state;
  uint8_t inverter_state;
  uint8_t relay_state;
  uint8_t inverter_run_mode_discharge_state;
  uint8_t inverter_command_mode;
  uint8_t inverter_enable;
  uint8_t direction_command;
} CAN_message_mc_internal_states_t;

#define RELAY_STATE_1(relay_state) ((relay_state) && 0x01)
#define RELAY_STATE_2(relay_state) ((relay_state) && 0x02 >> 1)
#define RELAY_STATE_3(relay_state) ((relay_state) && 0x04 >> 2)
#define RELAY_STATE_4(relay_state) ((relay_state) && 0x08 >> 3)
#define RELAY_STATE_5(relay_state) ((relay_state) && 0x10 >> 4)
#define RELAY_STATE_6(relay_state) ((relay_state) && 0x20 >> 5)
#define INVERTER_RUN_MODE(inverter_run_mode_discharge_state) ((inverter_run_mode_discharge_state) && 0x01)
#define INVERTER_ACTIVE_DISCHARGE_STATE(inverter_run_mode_discharge_state) ((inverter_run_mode_discharge_state) && 0xE0 >> 5)
#define INVERTER_ENABLE_STATE(inverter_enable) ((inverter_enable) && 0x01)
#define INVERTER_ENABLE_LOCKOUT(inverter_enable) ((inverter_enable) && 0x80 >> 7)

class MC_internal_states {
  public:
    MC_internal_states();
    MC_internal_states(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    uint8_t get_vsm_state();
    uint8_t get_inverter_state();
    bool get_relay_active_1();
    bool get_relay_active_2();
    bool get_relay_active_3();
    bool get_relay_active_4();
    bool get_relay_active_5();
    bool get_relay_active_6();
    bool get_inverter_run_mode();
    uint8_t get_inverter_active_discharge_state();
    bool get_inverter_command_mode();
    bool get_inverter_enable_state();
    bool get_inverter_enable_lockout();
    bool get_direction_command();
  private:
    CAN_message_mc_internal_states_t message;
};

typedef struct CAN_message_mc_fault_codes_t {
  uint16_t post_fault_lo;
  uint16_t post_fault_hi;
  uint16_t run_fault_lo;
  uint16_t run_fault_hi;
} CAN_message_mc_fault_codes_t;

#define POST_FAULT_LO_HW_GATE_DESATURATION_FAULT(post_fault_lo) ((post_fault_lo) && 0x0001)
#define POST_FAULT_LO_HW_OVERCURRENT_FAULT(post_fault_lo) ((post_fault_lo) && 0x0002 >> 1)
#define POST_FAULT_LO_ACCELERATOR_SHORTED(post_fault_lo) ((post_fault_lo) && 0x0004 >> 2)
#define POST_FAULT_LO_ACCELERATOR_OPEN(post_fault_lo) ((post_fault_lo) && 0x0008 >> 3)
#define POST_FAULT_LO_CURRENT_SENSOR_LOW(post_fault_lo) ((post_fault_lo) && 0x0010 >> 4)
#define POST_FAULT_LO_CURRENT_SENSOR_HIGH(post_fault_lo) ((post_fault_lo) && 0x0020 >> 5)
#define POST_FAULT_LO_MODULE_TEMP_LOW(post_fault_lo) ((post_fault_lo) && 0x0040 >> 6)
#define POST_FAULT_LO_MODULE_TEMP_HIGH(post_fault_lo) ((post_fault_lo) && 0x0080 >> 7)
#define POST_FAULT_LO_CTRL_PCB_TEMP_LOW(post_fault_lo) ((post_fault_lo) && 0x0100 >> 8)
#define POST_FAULT_LO_CTRL_PCB_TEMP_HIGH(post_fault_lo) ((post_fault_lo) && 0x0200 >> 9)
#define POST_FAULT_LO_GATE_DRIVE_PCB_TEMP_LOW(post_fault_lo) ((post_fault_lo) && 0x0400 >> 10)
#define POST_FAULT_LO_GATE_DRIVE_PCB_TEMP_HIGH(post_fault_lo) ((post_fault_lo) && 0x0800 >> 11)
#define POST_FAULT_LO_5V_SENSE_VOLTAGE_LOW(post_fault_lo) ((post_fault_lo) && 0x1000 >> 12)
#define POST_FAULT_LO_5V_SENSE_VOLTAGE_HIGH(post_fault_lo) ((post_fault_lo) && 0x2000 >> 13)
#define POST_FAULT_LO_12V_SENSE_VOLTAGE_LOW(post_fault_lo) ((post_fault_lo) && 0x4000 >> 14)
#define POST_FAULT_LO_12V_SENSE_VOLTAGE_HIGH(post_fault_lo) ((post_fault_lo) && 0x8000 >> 15)

class MC_fault_codes {
  public:
    MC_fault_codes(uint8_t buf[8]);
    void update(uint8_t buf[8]);
    bool get_lo_hw_gate_desaturation_fault();
    bool get_lo_hw_overcurrent_fault();
    bool get_lo_accelerator_shorted();
    bool get_lo_accelerator_open();
    bool get_lo_current_sensor_low();
    bool get_lo_current_sensor_high();
    bool get_lo_module_temp_low();
    bool get_lo_module_temp_high();
    bool get_lo_ctrl_pcb_temp_low();
    bool get_lo_ctrl_pcb_temp_high();
    bool get_lo_gate_drive_pcb_temp_low();
    bool get_lo_gate_drive_pcb_temp_high();
    bool get_lo_5v_sense_voltage_low();
    bool get_lo_5v_sense_voltage_high();
    bool get_lo_12v_sense_voltage_low();
    bool get_lo_12v_sense_voltage_high();
  private:
    CAN_message_mc_fault_codes_t message;
};

#endif
