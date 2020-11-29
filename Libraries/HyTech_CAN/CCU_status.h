#pragma once
#include <string.h>
#include <stdint.h>

#pragma pack(push,1)

class CCU_status {
public:
    CCU_status() = default;
    CCU_status(uint8_t buf[]) { load(buf); }

    inline void load(uint8_t buf[])     { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])    { memcpy(buf, this, sizeof(*this)); }

    inline bool get_charger_enabled() { return charger_enabled; }
    inline void set_charger_enabled(bool charger_enabled) { this->charger_enabled = charger_enabled; }
private:
    bool charger_enabled;
};

#pragma pack(pop)