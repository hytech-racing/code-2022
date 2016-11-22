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
#define ID_MC_TORQUE_TIMER_INFORMATION 0xAC
#define ID_MC_MODULATION_INDEX_FLUX_WEAKENING_OUTPUT_INFORMATION 0xAD
#define ID_MC_FIRMWARE_INFORMATION 0xAE
#define ID_MC_DIAGNOSTIC_DATA 0xAF
#define ID_MC_COMMAND_MESSAGE 0xC0

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

class MC_fault_codes {
  public:
    MC_fault_codes();
    MC_fault_codes(uint8_t buf[8]);
    void load(uint8_t buf[8]);
    bool get_post_lo_hw_gate_desaturation_fault();
    bool get_post_lo_hw_overcurrent_fault();
    bool get_post_lo_accelerator_shorted();
    bool get_post_lo_accelerator_open();
    bool get_post_lo_current_sensor_low();
    bool get_post_lo_current_sensor_high();
    bool get_post_lo_module_temp_low();
    bool get_post_lo_module_temp_high();
    bool get_post_lo_ctrl_pcb_temp_low();
    bool get_post_lo_ctrl_pcb_temp_high();
    bool get_post_lo_gate_drive_pcb_temp_low();
    bool get_post_lo_gate_drive_pcb_temp_high();
    bool get_post_lo_5v_sense_voltage_low();
    bool get_post_lo_5v_sense_voltage_high();
    bool get_post_lo_12v_sense_voltage_low();
    bool get_post_lo_12v_sense_voltage_high();
    bool get_post_hi_25v_sense_voltage_low();
    bool get_post_hi_25v_sense_voltage_high();
    bool get_post_hi_15v_sense_voltage_low();
    bool get_post_hi_15v_sense_voltage_high();
    bool get_post_hi_dc_bus_voltage_high();
    bool get_post_hi_dc_bus_voltage_low();
    bool get_post_hi_precharge_timeout();
    bool get_post_hi_precharge_voltage_failure();
    bool get_post_hi_eeprom_checksum_invalid();
    bool get_post_hi_eeprom_data_out_of_range();
    bool get_post_hi_eeprom_update_required();
    bool get_post_hi_reserved1();
    bool get_post_hi_reserved2();
    bool get_post_hi_reserved3();
    bool get_post_hi_brake_shorted();
    bool get_post_hi_brake_open();
    // TODO run faults
  private:
    CAN_message_mc_fault_codes_t message;
};

#endif
