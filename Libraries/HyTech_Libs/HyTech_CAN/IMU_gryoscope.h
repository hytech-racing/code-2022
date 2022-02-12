#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// @Parseclass
class IMU_gyroscope {
public:
    IMU_gyroscope() = default;
    IMU_gyroscope(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[8]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    const { memcpy(buf, this, sizeof(*this)); }

    //Getters
    inline int16_t get_yaw() const { return yaw; }
    inline int16_t get_pitch() const { return pitch; }
    inline int16_t get_roll() const { return roll; }

private:
    // @Parse @Unit(DEG/S) @Scale(3)
    int16_t yaw;
    // @Parse @Unit(DEG/S) @Scale(3)
    int16_t pitch;
    // @Parse @Unit(DEG/S) @Scale(3)
    int16_t roll;

};

#pragma pack(pop)