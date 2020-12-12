#pragma once
#include <string.h>
#include <cstdint>

#pragma pack(push,1)

/**
 * @Parseclass
 * @ID(TCU_WHEEL_RPM_FRONT, TCU_wheel_rpm_front) @Prefix(front, TCU_WHEEL_RPM_FRONT)
 * @ID(TCU_WHEEL_RPM_REAR, TCU_wheel_rpm_rear) @Prefix(rear, TCU_WHEEL_RPM_REAR)
 */
class TCU_wheel_rpm {
public:
    TCU_wheel_rpm() = default;
    TCU_wheel_rpm(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_wheel_rpm_left()     const { return wheel_rpm_left; }
    inline int16_t get_wheel_rpm_right()    const { return wheel_rpm_right; }

    inline void set_wheel_rpm_left(uint16_t value)  { this->wheel_rpm_left = value; }
    inline void set_wheel_rpm_right(uint16_t value) { this->wheel_rpm_right = value; }

private:
    int16_t wheel_rpm_left; // @Parse @Scale(100) @Unit(RPM)
    int16_t wheel_rpm_right; // @Parse @Scale(100) @Unit(RPM)
};

#pragma pack(pop)