/*
 * MC_voltage_information.cpp - CAN message parser: RMS Motor Controller voltage information message
 * Created by Nathan Cheek, November 20, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_voltage_information {
public:
    MC_voltage_information() = default;
    MC_voltage_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_dc_bus_voltage() { return dc_bus_voltage; }
    inline int16_t get_output_voltage() { return output_voltage; }
    inline int16_t get_phase_ab_voltage() { return phase_ab_voltage; }
    inline int16_t get_phase_bc_voltage() { return phase_bc_voltage; }
private:
    int16_t dc_bus_voltage;
    int16_t output_voltage;
    int16_t phase_ab_voltage;
    int16_t phase_bc_voltage;
};

#pragma pack(pop)