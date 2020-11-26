#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

class MC_fault_codes {
public:
    MC_fault_codes() = default;
    MC_fault_codes(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    uint16_t get_post_fault_lo() { return post_fault_lo; }
    uint16_t get_post_fault_hi() { return post_fault_hi; }
    uint16_t get_run_fault_lo() { return run_fault_lo; }
    uint16_t get_run_fault_hi() { return run_fault_hi; }

    inline bool get_post_lo_hw_gate_desaturation_fault()                const { return post_fault_lo & 0x0001; }
    inline bool get_post_lo_hw_overcurrent_fault()                      const { return post_fault_lo & 0x0002; }
    inline bool get_post_lo_accelerator_shorted()                       const { return post_fault_lo & 0x0004; }
    inline bool get_post_lo_accelerator_open()                          const { return post_fault_lo & 0x0008; }
    inline bool get_post_lo_current_sensor_low()                        const { return post_fault_lo & 0x0010; }
    inline bool get_post_lo_current_sensor_high()                       const { return post_fault_lo & 0x0020; }
    inline bool get_post_lo_module_temperature_low()                    const { return post_fault_lo & 0x0040; }
    inline bool get_post_lo_module_temperature_high()                   const { return post_fault_lo & 0x0080; }
    inline bool get_post_lo_ctrl_pcb_temperature_low()                  const { return post_fault_lo & 0x0100; }
    inline bool get_post_lo_ctrl_pcb_temperature_high()                 const { return post_fault_lo & 0x0200; }
    inline bool get_post_lo_gate_drive_pcb_temperature_low()            const { return post_fault_lo & 0x0400; }
    inline bool get_post_lo_gate_drive_pcb_temperature_high()           const { return post_fault_lo & 0x0800; }
    inline bool get_post_lo_5v_sense_voltage_low()                      const { return post_fault_lo & 0x1000; }
    inline bool get_post_lo_5v_sense_voltage_high()                     const { return post_fault_lo & 0x2000; }
    inline bool get_post_lo_12v_sense_voltage_low()                     const { return post_fault_lo & 0x4000; }
    inline bool get_post_lo_12v_sense_voltage_high()                    const { return post_fault_lo & 0x8000; }

    inline bool get_post_hi_25v_sense_voltage_low()                     const { return post_fault_hi & 0x0001; }
    inline bool get_post_hi_25v_sense_voltage_high()                    const { return post_fault_hi & 0x0002; }
    inline bool get_post_hi_15v_sense_voltage_low()                     const { return post_fault_hi & 0x0004; }
    inline bool get_post_hi_15v_sense_voltage_high()                    const { return post_fault_hi & 0x0008; }
    inline bool get_post_hi_dc_bus_voltage_high()                       const { return post_fault_hi & 0x0010; }
    inline bool get_post_hi_dc_bus_voltage_low()                        const { return post_fault_hi & 0x0020; }
    inline bool get_post_hi_precharge_timeout()                         const { return post_fault_hi & 0x0040; }
    inline bool get_post_hi_precharge_voltage_failure()                 const { return post_fault_hi & 0x0080; }
    inline bool get_post_hi_eeprom_checksum_invalid()                   const { return post_fault_hi & 0x0100; }
    inline bool get_post_hi_eeprom_data_out_of_range()                  const { return post_fault_hi & 0x0200; }
    inline bool get_post_hi_eeprom_update_required()                    const { return post_fault_hi & 0x0400; }
    inline bool get_post_hi_reserved1()                                 const { return post_fault_hi & 0x0800; } // TODO delete these?
    inline bool get_post_hi_reserved2()                                 const { return post_fault_hi & 0x1000; }
    inline bool get_post_hi_reserved3()                                 const { return post_fault_hi & 0x2000; }
    inline bool get_post_hi_brake_shorted()                             const { return post_fault_hi & 0x4000; }
    inline bool get_post_hi_brake_open()                                const { return post_fault_hi & 0x8000; }

    inline bool get_run_lo_motor_overspeed_fault()                      const { return run_fault_lo & 0x0001; }
    inline bool get_run_lo_overcurrent_fault()                          const { return run_fault_lo & 0x0002; }
    inline bool get_run_lo_overvoltage_fault()                          const { return run_fault_lo & 0x0004; }
    inline bool get_run_lo_inverter_overtemperature_fault()             const { return run_fault_lo & 0x0008; }
    inline bool get_run_lo_accelerator_input_shorted_fault()            const { return run_fault_lo & 0x0010; }
    inline bool get_run_lo_accelerator_input_open_fault()               const { return run_fault_lo & 0x0020; }
    inline bool get_run_lo_direction_command_fault()                    const { return run_fault_lo & 0x0040; }
    inline bool get_run_lo_inverter_response_timeout_fault()            const { return run_fault_lo & 0x0080; }
    inline bool get_run_lo_hardware_gatedesaturation_fault()            const { return run_fault_lo & 0x0100; }
    inline bool get_run_lo_hardware_overcurrent_fault()                 const { return run_fault_lo & 0x0200; }
    inline bool get_run_lo_undervoltage_fault()                         const { return run_fault_lo & 0x0400; }
    inline bool get_run_lo_can_command_message_lost_fault()             const { return run_fault_lo & 0x0800; }
    inline bool get_run_lo_motor_overtemperature_fault()                const { return run_fault_lo & 0x1000; }
    inline bool get_run_lo_reserved1()                                  const { return run_fault_lo & 0x2000; } // TODO delete these?
    inline bool get_run_lo_reserved2()                                  const { return run_fault_lo & 0x4000; }
    inline bool get_run_lo_reserved3()                                  const { return run_fault_lo & 0x8000; }

    inline bool get_run_hi_brake_input_shorted_fault()                  const { return run_fault_hi & 0x0001; }
    inline bool get_run_hi_brake_input_open_fault()                     const { return run_fault_hi & 0x0002; }
    inline bool get_run_hi_module_a_overtemperature_fault()             const { return run_fault_hi & 0x0004; }
    inline bool get_run_hi_module_b_overtemperature_fault()             const { return run_fault_hi & 0x0008; }
    inline bool get_run_hi_module_c_overtemperature_fault()             const { return run_fault_hi & 0x0010; }
    inline bool get_run_hi_pcb_overtemperature_fault()                  const { return run_fault_hi & 0x0020; }
    inline bool get_run_hi_gate_drive_board_1_overtemperature_fault()   const { return run_fault_hi & 0x0040; }
    inline bool get_run_hi_gate_drive_board_2_overtemperature_fault()   const { return run_fault_hi & 0x0080; }
    inline bool get_run_hi_gate_drive_board_3_overtemperature_fault()   const { return run_fault_hi & 0x0100; }
    inline bool get_run_hi_current_sensor_fault()                       const { return run_fault_hi & 0x0200; }
    inline bool get_run_hi_reserved1()                                  const { return run_fault_hi & 0x0400; } // TODO delete these?
    inline bool get_run_hi_reserved2()                                  const { return run_fault_hi & 0x0800; }
    inline bool get_run_hi_reserved3()                                  const { return run_fault_hi & 0x1000; }
    inline bool get_run_hi_reserved4()                                  const { return run_fault_hi & 0x2000; }
    inline bool get_run_hi_resolver_not_connected()                     const { return run_fault_hi & 0x4000; }
    inline bool get_run_hi_inverter_discharge_active()                  const { return run_fault_hi & 0x8000; }

private:
    uint16_t post_fault_lo;
    uint16_t post_fault_hi;
    uint16_t run_fault_lo;
    uint16_t run_fault_hi;
};

#pragma pack(pop)