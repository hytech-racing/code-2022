#pragma once
#include <string.h>
#include <stdint.h>
#ifdef HT_DEBUG_EN
    #include "Arduino.h"
#endif

#pragma pack(push,1)

// @Parseclass
class MC_firmware_information {
public:
    MC_firmware_information() = default;
    MC_firmware_information(uint8_t buf[8]) { load(buf); }

    inline void load(uint8_t buf[])         { memcpy(this, buf, sizeof(*this)); }
    inline void write(uint8_t buf[])  const { memcpy(buf, this, sizeof(*this)); }

    inline uint16_t get_eeprom_version_project_code()   const { return eeprom_version_project_code; }
    inline uint16_t get_software_version()              const { return software_version; }
    inline uint16_t get_date_code_mmdd()                const { return date_code_mmdd; }
    inline uint16_t get_date_code_yyyy()                const { return date_code_yyyy; }

#ifdef HT_DEBUG_EN
    void print() {
        Serial.println("\n\nMC FIRMWARE INFORMATION");
        Serial.println    ("-----------------------");
        Serial.print("EEPROM VERSION PROJECT CODE: 0x");    Serial.println(eeprom_version_project_code);
        Serial.print("SOFTWARE VERSION:            ");      Serial.println(software_version);
        Serial.print("DATE CODE MM/DD:             ");      Serial.println(date_code_mmdd);
        Serial.print("DATE CODE YYYY:              ");      Serial.println(date_code_yyyy);
    }
#endif

private:
    uint16_t eeprom_version_project_code;   // @Parse
    uint16_t software_version;              // @Parse
    uint16_t date_code_mmdd;                // @Parse
    uint16_t date_code_yyyy;                // @Parse
};

#pragma pack(pop)