#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass @Prefix(IC_{get_ic_id()}) @Custom(parse_detailed_voltages) @Indexable(get_ic_id(8), get_group_id(3))
class BMS_detailed_voltages {
public:
    BMS_detailed_voltages() = default;
    BMS_detailed_voltages(uint8_t buf[]) { load(buf); }
    BMS_detailed_voltages(uint8_t ic_id, uint8_t group_id, uint16_t voltage_0, uint16_t voltage_1, uint16_t voltage_2) {
        set_ic_id(ic_id);
        set_group_id(group_id);
        set_voltage_0(voltage_0);
        set_voltage_1(voltage_1);
        set_voltage_2(voltage_2);        
    }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline uint8_t get_ic_id()      const { return ic_id_group_id & 0xF; }
    inline uint8_t get_group_id()   const { return ic_id_group_id >> 4; }
    inline uint16_t get_voltage_0() const { return voltage_0; }
    inline uint16_t get_voltage_1() const { return voltage_1; }
    inline uint16_t get_voltage_2() const { return voltage_2; }
    uint16_t get_voltage(uint8_t voltage_id) const {
        switch(voltage_id) {
            case 0: return voltage_0;
            case 1: return voltage_1;
            case 2: return voltage_2;
        }
        return 0;
    }

    inline void set_ic_id(uint8_t ic_id)            { ic_id_group_id = (ic_id_group_id & 0xF0) | ic_id; }
    inline void set_group_id(uint8_t group_id)      { ic_id_group_id = (group_id << 4) | (ic_id_group_id & 0xF); }
    inline void set_voltage_0(uint16_t voltage_0)   { this->voltage_0 = voltage_0; };
    inline void set_voltage_1(uint16_t voltage_1)   { this->voltage_1 = voltage_1; };
    inline void set_voltage_2(uint16_t voltage_2)   { this->voltage_2 = voltage_2; };
    void set_voltage(uint8_t voltage_id, uint16_t voltage) {
        switch(voltage_id) {
            case 0: voltage_0 = voltage; return;
            case 1: voltage_1 = voltage; return;
            case 2: voltage_2 = voltage; return;
        }
    }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nBMS DETAILED VOLTAGES");
        Serial.println(    "---------------------");
        Serial.print("IC:        ");    Serial.println((uint32_t) get_ic_id());
        Serial.print("GROUP:     ");    Serial.println((uint32_t) get_group_id());
        Serial.print("VOLTAGE 0: ");    Serial.println(voltage_0 / 10000., 4);
        Serial.print("VOLTAGE 1: ");    Serial.println(voltage_1 / 10000., 4);
        Serial.print("VOLTAGE 2: ");    Serial.println(voltage_2 / 10000., 4);
    }
#endif

private:
	uint8_t ic_id_group_id;
    uint16_t voltage_0;
    uint16_t voltage_1;
    uint16_t voltage_2;
};

#pragma pack(pop)