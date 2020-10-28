/*
 * MC_modulation_index_flux_weakening_output_information.cpp - CAN message parser: RMS Motor Controller modulation index & flux weakening output information message
 * Created by Ryan Gallaway, December 1, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_modulation_index_flux_weakening_output_information {
public:
    MC_modulation_index_flux_weakening_output_information() = default;
    MC_modulation_index_flux_weakening_output_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_modulation_index() { return modulation_index; }
    inline int16_t get_flux_weakening_output() { return flux_weakening_output; }
    inline int16_t get_id_command() { return id_command; }
    inline int16_t get_iq_command() { return iq_command; }
private:
    uint16_t modulation_index; // TODO Signed or Unsigned?
    int16_t flux_weakening_output;
    int16_t id_command;
    int16_t iq_command;
};

#pragma pack(pop)