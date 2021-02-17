#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// @Parseclass
class MCU_GPS_readings {
public:
    MCU_GPS_readings() = default;
    MCU_GPS_readings(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline int32_t get_latitude()   const { return latitude; }
    inline int32_t get_longitude()  const { return longitude; }

    inline void set_latitude(int32_t latitude)      { this->latitude = latitude; }
    inline void set_longitude(int32_t longitude)    { this->longitude = longitude; }
private:
    int32_t latitude; // @Parse @Scale(10000)
    int32_t longitude; // @Parse @Scale(10000)
};

#pragma pack(pop)