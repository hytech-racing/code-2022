#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class MC_read_write_parameter_command {
public:
    MC_read_write_parameter_command() = default;
    MC_read_write_parameter_command(uint8_t buf[8]) { load(buf); };
    MC_read_write_parameter_command(uint16_t parameter_address, bool rw_command, uint32_t data) {
        set_parameter_address(parameter_address);
        set_rw_command(rw_command);
        set_data(data);
    }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_parameter_address() const { return parameter_address; }
    inline bool get_rw_command()            const { return rw_command; }
    inline uint32_t get_data()              const { return data; }

    inline void set_parameter_address(uint16_t parameter_address)   { this->parameter_address = parameter_address; }
    inline void set_rw_command(bool rw_command)                     { this->rw_command = rw_command; }
    inline void set_data(uint32_t data)                             { this->data = data; }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nMC READ WRITE PARAMETER COMMAND");
        Serial.println(    "-------------------------------");
        Serial.print("PARAMETER ADDRESS: ");  Serial.println(parameter_address, HEX);
        Serial.print("RW COMMAND:        ");  Serial.println(rw_command, HEX);
        Serial.print("DATA:              ");  Serial.println(data, HEX);
    }
#endif

private:
    uint16_t parameter_address; // @Parse @Hex
    bool rw_command;            // @Parse @Hex
    uint8_t reserved1;
    uint32_t data;              // @Parse @Hex
};

#pragma pack(pop)