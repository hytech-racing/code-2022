#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class MC_torque_timer_information {
public:
    MC_torque_timer_information() = default;
    MC_torque_timer_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline int16_t get_commanded_torque()   const { return commanded_torque; }
    inline int16_t get_torque_feedback()    const { return torque_feedback; }
    inline uint32_t get_power_on_timer()    const { return power_on_timer; }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nMC TORQUE TIMER INFORMATION");
        Serial.println(    "---------------------------");
        Serial.print("COMMANDED TORQUE: "); Serial.println(commanded_torque / 10.0, 1);
        Serial.print("TORQUE FEEDBACK:  "); Serial.println(torque_feedback);
        Serial.print("RMS UPTIME:       "); Serial.println(power_on_timer * .003, 0);
    }
#endif

private:
    int16_t commanded_torque; // @Parse @Scale(10) @Unit(C)
    int16_t torque_feedback; // @Parse @Scale(10) @Unit(C)
    uint32_t power_on_timer; // @Parse @Name(rms_uptime) @Unit(s)
};

#pragma pack(pop)