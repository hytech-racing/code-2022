#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass @Prefix(BMS)
class BMS_status {
public:
    BMS_status() = default;
    BMS_status(uint8_t buf[]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline uint8_t get_state()                   const { return state; }
    inline uint16_t get_error_flags()            const { return error_flags; }
    inline bool get_overvoltage()                const { return error_flags & 0x1; }    // @Parseflag(error_flags)
    inline bool get_undervoltage()               const { return error_flags & 0x2; }    // @Parseflag(error_flags)
    inline bool get_total_voltage_high()         const { return error_flags & 0x4; }    // @Parseflag(error_flags)
    inline bool get_discharge_overcurrent()      const { return error_flags & 0x8; }    // @Parseflag(error_flags)
    inline bool get_charge_overcurrent()         const { return error_flags & 0x10; }   // @Parseflag(error_flags)
    inline bool get_discharge_overtemp()         const { return error_flags & 0x20; }   // @Parseflag(error_flags)
    inline bool get_charge_overtemp()            const { return error_flags & 0x40; }   // @Parseflag(error_flags)
    inline bool get_undertemp()                  const { return error_flags & 0x80; }   // @Parseflag(error_flags)
    inline bool get_onboard_overtemp()           const { return error_flags & 0x100; }  // @Parseflag(error_flags)
    inline int16_t get_current()                 const { return current; }
    inline uint8_t get_flags()                   const { return flags; }
    inline bool get_shutdown_g_above_threshold() const { return flags & 0x1; }
    inline bool get_shutdown_h_above_threshold() const { return flags & 0x2; }

    inline void set_state(uint8_t state)                                        { this->state = state; }
    inline void set_error_flags(uint16_t error_flags)                           { this->error_flags = error_flags; }
    inline void set_overvoltage(bool overvoltage)                               { error_flags = (error_flags & 0xFFFE) | overvoltage; }
    inline void set_undervoltage(bool undervoltage)                             { error_flags = (error_flags & 0xFFFD) | (undervoltage          << 1); }
    inline void set_total_voltage_high(bool total_voltage_high)                 { error_flags = (error_flags & 0xFFFB) | (total_voltage_high    << 2); }
    inline void set_discharge_overcurrent(bool discharge_overcurrent)           { error_flags = (error_flags & 0xFFF7) | (discharge_overcurrent << 3); }
    inline void set_charge_overcurrent(bool charge_overcurrent)                 { error_flags = (error_flags & 0xFFEF) | (charge_overcurrent    << 4); }
    inline void set_discharge_overtemp(bool discharge_overtemp)                 { error_flags = (error_flags & 0xFFDF) | (discharge_overtemp    << 5); }
    inline void set_charge_overtemp(bool charge_overtemp)                       { error_flags = (error_flags & 0xFFBF) | (charge_overtemp       << 6); }
    inline void set_undertemp(bool undertemp)                                   { error_flags = (error_flags & 0xFF7F) | (undertemp             << 7); }
    inline void set_onboard_overtemp(bool onboard_overtemp)                     { error_flags = (error_flags & 0xFEFF) | (onboard_overtemp      << 8); }
    inline void set_current(int16_t current)                                    { this->current = current; }
    inline void set_flags(uint8_t flags)                                        { this->flags = flags; }
    inline void set_shutdown_g_above_threshold(bool shutdown_g_above_threshold) { flags = (flags & 0xFFFE) | shutdown_g_above_threshold; }
    inline void set_shutdown_h_above_threshold(bool shutdown_h_above_threshold) { flags = (flags & 0xFFFD) | (shutdown_h_above_threshold << 1); }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nBMS Status");
        Serial.println("----------");
        Serial.print("STATE:       ");      Serial.println(state, HEX);
        Serial.print("ERROR FLAGS: 0x");    Serial.println(error_flags, HEX);
        Serial.print("CURRENT:     ");      Serial.println(current / 100.0, 2);
        Serial.print("FLAGS:       0x");    Serial.println(flags, HEX);
    }
#endif

private:
	uint8_t state;          // @Parse @Hex
    uint16_t error_flags;   // @Parse @Flagset @Hex
    int16_t current;        // @Parse @Scale(100) @Unit(A)
    uint8_t flags;          // @Parse @Hex @Flaglist(shutdown_g_above_threshold, shutdown_h_above_threshold)
};

#pragma pack(pop)