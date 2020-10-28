/*
 * MCU_GPS_readings_alpha.cpp - CAN message parser: MCU_GPS Alpha message
 * Created by Nathan Cheek, May 19, 2019.
 */

#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

class MCU_GPS_readings {
public:
    MCU_GPS_readings() = default;
    MCU_GPS_readings(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[]) { memcpy(buf, this, sizeof(*this)); }

    inline int32_t get_latitude() { return latitude; }
    inline int32_t get_longitude() { return longitude; }

    inline void set_latitude(int32_t latitude) { this->latitude = latitude; }
    inline void set_longitude(int32_t longitude) { this->longitude = longitude; }
private:
    int32_t latitude;
    int32_t longitude;
};

#pragma pack(pop)