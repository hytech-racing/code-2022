#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// TODO: define @Parseclass
class IMU_gyroscope {
public:
    IMU_gyroscope() = default;
    IMU_gyroscope(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[8]) { memcpy(this, buf, sizeof(*this)); }

/*
    //Getters
    inline void get_yaw() const { return yaw; }
    inline void get_pitch() const { return pitch; }
    inline void get_roll() const { return roll; }

private:
    int16_t yaw = (((int16_t)(buf[1]) << 8) | buf[0]) * 360;
    int16_t pitch = (((int16_t)(buf[3]) << 8) | buf[2]) * 360;
    int16_t roll = (((int16_t)(buf[5]) << 8) | buf[4]) * 360;
*/

}