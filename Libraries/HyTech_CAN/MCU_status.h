#pragma once

#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

enum class MCU_STATE
{
    STARTUP                      = 0,
    TRACTIVE_SYSTEM_NOT_ACTIVE   = 1,
    TRACTIVE_SYSTEM_ACTIVE       = 2,
    ENABLING_INVERTER            = 3,
    WAITING_READY_TO_DRIVE_SOUND = 4,
    READY_TO_DRIVE               = 5
};

#pragma pack(push,1)

// @Parseclass @Custom(parse_mcu_enums)
class MCU_status {
public:
    MCU_status() = default;
    MCU_status(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[])  { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) const { memcpy(buf, this, sizeof(*this)); }

    /* Shutdown system monitoring */
    inline uint8_t get_shutdown_inputs()           const { return shutdown_states; }
    inline bool get_imd_ok_high()                  const { return shutdown_states & 0x01; }
    inline bool get_shutdown_b_above_threshold()   const { return shutdown_states & 0x02; }
    inline bool get_bms_ok_high()                  const { return shutdown_states & 0x04; }
    inline bool get_shutdown_c_above_threshold()   const { return shutdown_states & 0x08; }
    inline bool get_bspd_ok_high()                 const { return shutdown_states & 0x10; }
    inline bool get_shutdown_d_above_threshold()   const { return shutdown_states & 0x20; }
    inline bool get_software_ok_high()             const { return shutdown_states & 0x40; }
    inline bool get_shutdown_e_above_threshold()   const { return shutdown_states & 0x80; }

    inline void set_shutdown_inputs(const uint8_t inputs)        { shutdown_states = inputs; }
    inline void set_imd_ok_high(const bool high)                 { shutdown_states = (shutdown_states & 0xFE) | (high      ); }
    inline void set_shutdown_b_above_threshold(const bool above) { shutdown_states = (shutdown_states & 0xFD) | (above << 1); }
    inline void set_bms_ok_high(const bool high)                 { shutdown_states = (shutdown_states & 0xFB) | (high  << 2); }
    inline void set_shutdown_c_above_threshold(const bool above) { shutdown_states = (shutdown_states & 0xF7) | (above << 3); }
    inline void set_bspd_ok_high(const bool high)                { shutdown_states = (shutdown_states & 0xEF) | (high  << 4); }
    inline void set_shutdown_d_above_threshold(const bool above) { shutdown_states = (shutdown_states & 0xDF) | (above << 5); }
    inline void set_software_ok_high(const bool high)            { shutdown_states = (shutdown_states & 0xBF) | (high  << 6); }
    inline void set_shutdown_e_above_threshold(const bool above) { shutdown_states = (shutdown_states & 0x7F) | (above << 7); }

    /* Pedal system monitoring */

    inline uint8_t get_pedal_states()               const { return pedal_states; }
    inline bool get_no_accel_implausability()       const { return pedal_states & 0x04; }
    inline bool get_no_brake_implausability()       const { return pedal_states & 0x08; }
    inline bool get_brake_pedal_active()            const { return pedal_states & 0x10; }
    inline bool get_bspd_current_high()             const { return pedal_states & 0x20; }
    inline bool get_bspd_brake_high()               const { return pedal_states & 0x40; }
    inline bool get_no_accel_brake_implausability() const { return pedal_states & 0x80; }

    inline void set_pedal_states(const uint8_t states)                 { pedal_states = states; }
    inline void set_no_accel_implausability(const bool implausable)       { pedal_states = (pedal_states & 0xFB) | (implausable << 2); }
    inline void set_no_brake_implausability(const bool implausable)       { pedal_states = (pedal_states & 0xF7) | (implausable << 3); }
    inline void set_brake_pedal_active(const bool pressed)                { pedal_states = (pedal_states & 0xEF) | (pressed     << 4); }
    inline void set_bspd_current_high(const bool high)                    { pedal_states = (pedal_states & 0xDF) | (high        << 5); }
    inline void set_bspd_brake_high(const bool high)                      { pedal_states = (pedal_states & 0xBF) | (high        << 6); }
    inline void set_no_accel_brake_implausability(const bool implausable) { pedal_states = (pedal_states & 0x7F) | (implausable << 7); }

    /* ECU state */

    inline uint8_t get_ecu_states()        const { return (ecu_states); }
    inline MCU_STATE get_state()           const { return static_cast<MCU_STATE>((ecu_states & 0x07)); }
    inline bool get_inverter_powered()     const { return (ecu_states & 0x08); }
    inline bool get_energy_meter_present() const { return (ecu_states & 0x10); }
    inline bool get_activate_buzzer()      const { return (ecu_states & 0x20); }
    inline bool get_software_is_ok()       const { return (ecu_states & 0x40); }
    inline bool get_launch_ctrl_active()   const { return (ecu_states & 0x80); }

    inline void set_ecu_states(const uint8_t states)         { ecu_states = states; }
    inline void set_state(const MCU_STATE state)             { ecu_states = (ecu_states & 0xF8) | (static_cast<uint8_t>(state)); }
    inline void set_inverter_powered(const bool powered)     { ecu_states = (ecu_states & 0xF7) | (powered  << 3); }
    inline void set_energy_meter_present(const bool present) { ecu_states = (ecu_states & 0xEF) | (present  << 4); }
    inline void set_activate_buzzer(const bool activate)     { ecu_states = (ecu_states & 0xDF) | (activate << 5); }
    inline void set_software_is_ok(const bool is_ok)         { ecu_states = (ecu_states & 0xBF) | (is_ok    << 6); }
    inline void set_launch_ctrl_active(const bool active)    { ecu_states = (ecu_states & 0x7F) | (active   << 7); }

    inline void toggle_launch_ctrl_active() { ecu_states ^= 0x80; }

    /* distance travelled */
    inline uint16_t get_distance_travelled() const { return distance_travelled; }
    inline void set_distance_travelled(const uint16_t distance) { distance_travelled = distance; }


    inline uint8_t get_max_torque()         const { return max_torque; }
    inline void set_max_torque(const uint8_t max) { max_torque = max; }

    inline uint8_t get_torque_mode()          const { return torque_mode; }
    inline void set_torque_mode(const uint8_t mode) { torque_mode = mode; }

private:
    // no free bits
    /* @Parse @Flaglist(
         imd_ok_high,
         shutdown_b_above_threshold,
         bms_ok_high,
         shutdown_c_above_threshold,
         bspd_ok_high,
         shutdown_d_above_threshold,
         software_ok_high,
         shutdown_e_above_threshold
        ) */
    uint8_t shutdown_states;
    /*
     * torque_mode (2 bits)
     * accel_implausability
     * brake_implausability
     * brake_pressed
     * Current high
     * brake high
     * accel/brake implausability
     */
    /* @Parse @Flaglist(
        no_accel_implausability,
        no_brake_implausability,
        brake_pedal_active,
        bspd_current_high,
        bspd_brake_high,
        no_accel_brake_implausability
        ) */
    uint8_t pedal_states;

    /**
     * state (3 bits)
     * inverter powered
     * energy_meter
     * activate_buzzer
     * software_is_ok
     * launch_control_active
     */
    /* @Parse @Flaglist(
        inverter_powered,
        energy_meter_present,
        activate_buzzer,
        software_is_ok,
        launch_ctrl_active
        )*/
    uint8_t ecu_states;

    // @Parse @Unit(Nm)
    uint8_t max_torque;

    // @Parse
    uint8_t torque_mode;

    // @Parse @Unit(m) @Scale(100)
    uint16_t distance_travelled;
};

#pragma pack(pop)
