/*
 * MC_read_write_parameter_command.cpp - CAN message parser: RMS Motor Controller read / write parameter command message - sent to PM
 * Created by Nathan Cheek, November 22, 2016.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MC_read_write_parameter_command {
public:
    MC_read_write_parameter_command() = default;
    MC_read_write_parameter_command(uint8_t buf[8]) { load(buf); };
    MC_read_write_parameter_command(uint16_t parameter_address, bool rw_command, uint32_t data) {
        set_parameter_address(parameter_address);
        set_rw_command(rw_command);
        set_data(data);
    }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_parameter_address() { return parameter_address; }
    inline bool get_rw_command() { return rw_command; }
    inline uint32_t get_data() { return data; }

    inline void set_parameter_address(uint16_t parameter_address) { this->parameter_address = parameter_address; }
    inline void set_rw_command(bool rw_command) { this->rw_command = rw_command; }
    inline void set_data(uint32_t data) { this->data = data; }
private:
    uint16_t parameter_address;
    bool rw_command;
    uint8_t reserved1;
    uint32_t data;
} CAN_message_mc_read_write_parameter_command_t;

#pragma pack(pop)