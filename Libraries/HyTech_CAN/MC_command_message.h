#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

class MC_command_message {
public:
    MC_command_message() = default;
    MC_command_message(uint8_t buf[8]) { load(buf); }
    MC_command_message(int16_t torque_command, int16_t angular_velocity, bool direction, bool inverter_enable, bool discharge_enable, int16_t commanded_torque_limit) {
        set_torque_command(torque_command);
        set_angular_velocity(angular_velocity);
        set_direction(direction);
        set_inverter_enable(inverter_enable);
        set_discharge_enable(discharge_enable);
        set_commanded_torque_limit(commanded_torque_limit);
    }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_torque_command()         const { return torque_command; }
    inline int16_t get_angular_velocity()       const { return angular_velocity; }
    inline bool get_direction()                 const { return direction; }
    inline bool get_inverter_enable()           const { return inverter_enable_discharge_enable & 0x1; }
    inline bool get_discharge_enable()          const { return inverter_enable_discharge_enable & 0x2; }
    inline int16_t get_commanded_torque_limit() const { return commanded_torque_limit; }

    inline void set_torque_command(int16_t torque_command)                 { this->torque_command = torque_command; }
    inline void set_angular_velocity(int16_t angular_velocity)             { this->angular_velocity = angular_velocity; }
    inline void set_direction(bool direction)                              { this->direction = direction; }
    inline void set_inverter_enable(bool inverter_enable)                  { inverter_enable_discharge_enable = (inverter_enable_discharge_enable & 0xFE) | (inverter_enable); }
    inline void set_discharge_enable(bool discharge_enable)                { inverter_enable_discharge_enable = (inverter_enable_discharge_enable & 0xFD) | (discharge_enable << 1); }
    inline void set_commanded_torque_limit(int16_t commanded_torque_limit) { this->commanded_torque_limit = commanded_torque_limit; }

private:
    int16_t torque_command;
    int16_t angular_velocity;
    bool direction;
    uint8_t inverter_enable_discharge_enable;
    int16_t commanded_torque_limit;
};

#pragma pack(pop)