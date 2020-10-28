/*
 * MC_torque_timer_information.cpp - CAN message parser: RMS Motor Controller torque timer information message
 * Created by Nathan Cheek, November 22, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_torque_timer_information {
public:
    MC_torque_timer_information() = default;
    MC_torque_timer_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_commanded_torque() { return commanded_torque; }
    inline int16_t get_torque_feedback() { return torque_feedback; }
    inline uint32_t get_power_on_timer() { return power_on_timer; }
private:
    int16_t commanded_torque;
    int16_t torque_feedback;
    uint32_t power_on_timer;
};

#pragma pack(pop)