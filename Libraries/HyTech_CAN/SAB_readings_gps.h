#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// @Parseclass
class SAB_readings_gps {
public:
    SAB_readings_gps() = default;

    SAB_readings_gps(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[8]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[8]) const { memcpy(buf, this, sizeof(*this)); }

    // Getters
    inline int32_t get_gps_latitude() const { return gps_latitude; }
    inline int32_t get_gps_longitude() const { return gps_longitude; }

    // Setters
    inline void set_gps_latitude(int32_t reading) { gps_latitude = reading; }
    inline void set_gps_longitude(int32_t reading) { gps_longitude = reading; }

private:
    // @Parse @Scale(1000000)
    int32_t gps_latitude;
    // @Parse @Scale(1000000)
    int32_t gps_longitude;
};

#pragma pack(pop)