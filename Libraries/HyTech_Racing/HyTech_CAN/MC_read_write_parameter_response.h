#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class MC_read_write_parameter_response {
public:
    MC_read_write_parameter_response() = default;
    MC_read_write_parameter_response(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_parameter_address() const { return parameter_address; }
    inline bool get_write_success()         const { return write_success; }
    inline uint32_t get_data()              const { return data; }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nMC READ WRITE PARAMETER RESPONSE");
        Serial.println(    "--------------------------------");
        Serial.print("PARAMETER ADDRESS: ");  Serial.println(parameter_address, HEX);
        Serial.print("WRITE SUCCESS:     ");  Serial.println(write_success, HEX);
        Serial.print("DATA:              ");  Serial.println(data, HEX);
    }
#endif

private:
    uint16_t parameter_address; // @Parse @Hex
    bool write_success;         // @Parse
    uint8_t reserved1;
    uint32_t data;              // @Parse @Hex
};

#pragma pack(pop)