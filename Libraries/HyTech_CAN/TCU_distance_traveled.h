#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

class TCU_distance_traveled {
public:
    TCU_distance_traveled() = default;
    TCU_distance_traveled(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_distance_traveled()  const { return distance_traveled; }

    inline void set_distance_traveled(uint16_t value) { this->distance_traveled = distance_traveled; }
private:
    int16_t distance_traveled;
};

#pragma pack(pop)