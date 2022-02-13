#pragma once

#include <stdint.h>
#include <string.h>

#pragma pack(push,1)

// @Parseclass
class MCU_wheel_speed {
public:
    MCU_wheel_speed() = default;

    MCU_wheel_speed(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[8])  { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[8]) const { memcpy(buf, this, sizeof(*this)); }

    // Getters
    inline uint16_t get_rpm_front_left()  const { return rpm_front_left; }
    inline uint16_t get_rpm_front_right() const { return rpm_front_right; }
    inline uint16_t get_rpm_back_left()   const { return rpm_back_left; }
    inline uint16_t get_rpm_back_right()  const { return rpm_back_right; }

    // Setters
    inline void set_rpm_front_left(uint16_t speed)  { rpm_front_left = speed; }
    inline void set_rpm_front_right(uint16_t speed) { rpm_front_right = speed; }
    inline void set_rpm_back_left(uint16_t speed)   { rpm_back_left = speed; }
    inline void set_rpm_back_right(uint16_t speed)  { rpm_back_right = speed; }

private:
    // @Parse @Unit(rpm) @Scale(10)
    uint16_t rpm_front_left;
    // @Parse @Unit(rpm) @Scale(10)
    uint16_t rpm_front_right;
    // @Parse @Unit(rpm) @Scale(10)
    uint16_t rpm_back_left;
    // @Parse @Unit(rpm) @Scale(10)
    uint16_t rpm_back_right;   
};

#pragma pack(pop)