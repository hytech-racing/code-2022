/*
 * MC_internal_states.cpp - CAN message parser: RMS Motor Controller internal states message
 * Created by Nathan Cheek, November 20, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_internal_states {
public:
    MC_internal_states() = default;
    MC_internal_states(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline uint8_t get_vsm_state()                          const { return vsm_state; }
    inline uint8_t get_inverter_state()                     const { return inverter_state; }
    inline bool get_relay_active_1()                        const { return relay_state & 0x01; }
    inline bool get_relay_active_2()                        const { return relay_state & 0x02; }
    inline bool get_relay_active_3()                        const { return relay_state & 0x04; }
    inline bool get_relay_active_4()                        const { return relay_state & 0x08; }
    inline bool get_relay_active_5()                        const { return relay_state & 0x10; }
    inline bool get_relay_active_6()                        const { return relay_state & 0x20; }
    inline bool get_inverter_run_mode()                     const { return inverter_run_mode_discharge_state & 1; }
    inline uint8_t get_inverter_active_discharge_state()    const { return inverter_run_mode_discharge_state >> 5; }
    inline bool get_inverter_command_mode()                 const { return inverter_command_mode; }
    inline bool get_inverter_enable_state()                 const { return inverter_enable & 1; }
    inline bool get_inverter_enable_lockout()               const { return inverter_enable & 0x80; }
    inline bool get_direction_command()                     const { return direction_command; }
private:
    uint16_t vsm_state;
    uint8_t inverter_state;
    uint8_t relay_state;
    uint8_t inverter_run_mode_discharge_state;
    uint8_t inverter_command_mode;
    uint8_t inverter_enable;
    uint8_t direction_command;
};

#pragma pack(pop)