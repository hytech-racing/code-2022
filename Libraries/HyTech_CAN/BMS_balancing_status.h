/*
 * 5-byte message structure from MSB to LSB
 * [3 bits: empty] [1 bit: group][9 bits: IC 4 balancing data] [9 bits: IC 3 balancing data] [9 bits: IC 2 balancing data] [9 bits: IC 1 balancing data] 
 * 
 * Structure of each IC section from MSB to LSB
 * [Cell 9 balancing] [Cell 8 balancing] [Cell 7 balancing] [Cell 6 balancing] [Cell 5 balancing] [Cell 4 balancing] [Cell 3 balancing] [Cell 2 balancing] [Cell 1 balancing]
 */

#pragma once

#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class BMS_balancing_status {
    private:
        uint64_t message;
    public:
        BMS_balancing_status() = default;
        BMS_balancing_status(uint8_t buf[]) { load(buf); }
        BMS_balancing_status(uint8_t group_id, int64_t balancing_status) : BMS_balancing_status() { 
            set_group_id(group_id);
            set_balancing(balancing_status);
        }

        inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
        inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

        inline uint8_t get_group_id()   { return message & 0xF; }
        inline uint64_t get_balancing() { return message >> 0x4; }
        inline uint16_t get_ic_balancing(uint8_t ic_id) { return (message >> (0x4 + 0x9 * ic_id)) & 0x1FF; }
        inline bool get_cell_balancing(uint8_t ic_id, uint16_t cell_id) { return (get_ic_balancing(ic_id) >> cell_id) & 0x1; }

        inline void set_group_id(uint8_t group_id) { message = (message & 0xFFFFFFFFF0) | (group_id & 0xF); }
        inline void set_balancing(uint64_t balancing_status) { message = (message & 0xF) | (balancing_status << 4); }
        inline void set_ic_balancing(uint8_t ic_id, uint16_t balancing_status) {
            int shift = 4 + 9 * ic_id;
            message &= ~(((uint64_t) 0x1FF) << shift);
            message |= ((uint64_t) balancing_status & 0x1FF) << shift;
        }
        inline void set_cell_balancing(uint8_t ic_id, uint8_t cell_id, bool balancing_status) {
            int shift = 4 + 9 * ic_id + cell_id;
            message &= ~(((uint64_t) 0x1) << shift);
            message |= balancing_status << shift;
        }
};

#pragma pack(pop)
