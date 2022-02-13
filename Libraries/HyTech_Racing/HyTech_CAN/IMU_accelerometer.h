#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// @Parseclass
class IMU_accelerometer {
public:
    IMU_accelerometer() = default;
    IMU_accelerometer(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[8]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    const { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_lat_accel() const { return lat_accel; }
    inline int16_t get_long_accel() const { return long_accel; }
    inline int16_t get_vert_accel() const { return vert_accel; }

private:
    // @Parse @Unit(m/s/s) @Scale(102)
    int16_t lat_accel;
    // @Parse @Unit(m/s/s) @Scale(102)
    int16_t long_accel;
    // @Parse @Unit(m/s/s) @Scale(102)
    int16_t vert_accel;
    
};

#pragma pack(pop)