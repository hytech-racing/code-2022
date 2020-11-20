#pragma once

#include <bitset>
#include <fstream>
#include <sstream>

#include "HTException.h"
#include "MockPin.h"

#define HEX 16

class MockSerial {
public:
    MockSerial(int id);
    ~MockSerial();
    void begin(unsigned int baudRate);
    void end();
    void setOutputPath(std::string filepath);
    template <typename T> inline void print(T value) { validate(); *fos << value; }
    template <typename T> inline void print(T value, int base) { validate(); *fos << format(value, base).rdbuf(); }
    void println() { validate(); *fos << '\n'; }
    template <typename T> inline void println(T value) { validate(); *fos << value << '\n'; }
    template <typename T> inline void println(T value, int base) { validate(); *fos << format(value, base).rdbuf() << '\n'; }
    void write(uint8_t* buf, int size);
private:
    int fId;
    std::string fFilepath;
    std::ostream *fos = nullptr;
    inline void validate() { if (!fos) throw InvalidPinConfigurationException(-1, OUTPUT, UNUSED); }

    template <typename T> inline std::stringstream format(T value, int base) {
        std::stringstream ss;
        if (base == 2) ss << std::bitset<sizeof(value) << 3>(value);
        else if (base == 8) ss << std::oct << value;
        else if (base == 16) ss << std::hex << value;
        else ss << value;
        return ss;
    }
};

extern MockSerial Serial, Serial2;