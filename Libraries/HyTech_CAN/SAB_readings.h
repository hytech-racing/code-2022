#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

// @Parseclass @ID(ID_SAB_REAR, SAB_REAR) @ID(ID_SAB_FRONT, SAB_FRONT)
class SAB_readings {
public:
    SAB_readings() = default;

    SAB_readings(const uint8_t buf[8]) { load(buf); }

    inline void load(const uint8_t buf[8]) { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[8]) const { memcpy(buf, this, sizeof(*this)); }

    // Getters
    inline uint16_t get_sensor_1() const { return sensor_1; }
    inline uint16_t get_sensor_2() const { return sensor_2; }
    inline uint16_t get_sensor_3() const { return sensor_3; }
    inline uint16_t get_sensor_4() const { return sensor_4; }

    // Setters
    inline void set_sensor_1(uint16_t reading) { sensor_1 = reading; }
    inline void set_sensor_2(uint16_t reading) { sensor_2 = reading; }
    inline void set_sensor_3(uint16_t reading) { sensor_3 = reading; }
    inline void set_sensor_4(uint16_t reading) { sensor_4 = reading; }

private:
    // @Parse
    uint16_t sensor_1;
    // @Parse
    uint16_t sensor_2;
    // @Parse
    uint16_t sensor_3;
    // @Parse
    uint16_t sensor_4;
};

#pragma pack(pop)