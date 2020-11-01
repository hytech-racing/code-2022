#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MCU_pedal_readings {
public:
    MCU_pedal_readings() = default;
    MCU_pedal_readings(uint8_t buf[8]) { load(buf); }
    MCU_pedal_readings(uint16_t accelerator_pedal_raw_1, uint16_t accelerator_pedal_raw_2, uint16_t brake_pedal_raw, uint8_t pedal_flags, uint8_t torque_map_mode) {
        set_accelerator_pedal_raw_1(accelerator_pedal_raw_1);
        set_accelerator_pedal_raw_2(accelerator_pedal_raw_2);
        set_brake_pedal_raw(brake_pedal_raw);
        set_pedal_flags(pedal_flags);
        set_torque_map_mode(torque_map_mode);
    }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_accelerator_pedal_raw_1()   const { return accelerator_pedal_raw_1; }
    inline uint16_t get_accelerator_pedal_raw_2()   const { return accelerator_pedal_raw_2; }
    inline uint16_t get_brake_pedal_raw()           const { return brake_pedal_raw; }
    inline uint8_t  get_pedal_flags()               const { return pedal_flags; }

    inline bool get_accelerator_implausibility()    const { return pedal_flags & 0x01; }
    inline bool get_brake_implausibility()          const { return pedal_flags & 0x02; }
    inline bool get_brake_pedal_active()            const { return pedal_flags & 0x04; }
    inline uint8_t get_torque_map_mode()            const { return torque_map_mode; }

    inline void set_accelerator_pedal_raw_1(uint16_t accelerator_pedal_raw_1)   { this->accelerator_pedal_raw_1 = accelerator_pedal_raw_1; }
    inline void set_accelerator_pedal_raw_2(uint16_t accelerator_pedal_raw_2)   { this->accelerator_pedal_raw_2 = accelerator_pedal_raw_2; }
    inline void set_brake_pedal_raw(uint16_t brake_pedal_raw)                   { this->brake_pedal_raw = brake_pedal_raw; }
    inline void set_pedal_flags(uint8_t pedal_flags)                            { this->pedal_flags = pedal_flags; }
    inline void set_accelerator_implausibility(bool accelerator_implausibility) { pedal_flags = (pedal_flags & 0xFE) | accelerator_implausibility; }
    inline void set_brake_implausibility(bool brake_implausibility)             { pedal_flags = (pedal_flags & 0xFD) | (brake_implausibility << 1); }
    inline void set_brake_pedal_active(bool brake_pedal_active)                 { pedal_flags = (pedal_flags & 0xFB) | (brake_pedal_active << 2); }
    inline void set_torque_map_mode(uint8_t torque_map_mode)                    { this->torque_map_mode = torque_map_mode; }
private:
    uint16_t accelerator_pedal_raw_1;
    uint16_t accelerator_pedal_raw_2;
    uint16_t brake_pedal_raw;
    uint8_t pedal_flags;
    uint8_t torque_map_mode;
} CAN_message_mcu_pedal_readings_t;

#pragma pack(pop)