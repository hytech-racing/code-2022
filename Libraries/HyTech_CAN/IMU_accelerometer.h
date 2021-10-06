#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// TODO: define @Parseclass
class IMU_accelerometer {
public:
    IMU_accelerometer() = default;
    IMU_accelerometer(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[8]) { memcpy(this, buf, sizeof(*this)); }

    //Getters
    inline void get_lat_accel() const { return lat_accel; }
    inline void get_long_accel() const { return long_accel; }
    inline void get_vert_accel() const { return vert_accel; }

private:
    int16_t lat_accel = ((int16_t)(buf[1]) << 8) | buf[0];
    int16_t long_accel = ((int16_t)(buf[3]) << 8) | buf[2];
    int16_t vert_accel = ((int16_t)(buf[5]) << 8) | buf[4];
}