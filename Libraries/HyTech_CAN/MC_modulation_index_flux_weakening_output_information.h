#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// @Parseclass
class MC_modulation_index_flux_weakening_output_information {
public:
    MC_modulation_index_flux_weakening_output_information() = default;
    MC_modulation_index_flux_weakening_output_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_modulation_index()     const { return modulation_index; }
    inline int16_t get_flux_weakening_output() const { return flux_weakening_output; }
    inline int16_t get_id_command()            const { return id_command; }
    inline int16_t get_iq_command()            const { return iq_command; }
private:
    uint16_t modulation_index; // @Parse @Hex TODO Signed or Unsigned?
    int16_t flux_weakening_output; // @Parse @Hex
    int16_t id_command; // @Parse @Hex
    int16_t iq_command; // @Parse @Hex
};

#pragma pack(pop)