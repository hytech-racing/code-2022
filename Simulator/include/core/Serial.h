#pragma once

#include <bitset>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "BoardDef.h"
#include "HTException.h"
#include "MockPin.h"

#define HEX 16

class MockSerial {
public:
    MockSerial(int id) : fId(id), internal_comm_en(false), fout(nullptr) {}
    ~MockSerial();
    void begin(unsigned int baudRate, uint8_t mode = std::ios::ios_base::out);
    void end();
    void setOutputPath(std::string filepath);
    inline void print(double value, int precision) { vout() << std::setprecision(precision) << value; }
    template <typename T> inline void print(T value) { vout() << value; }
    template <typename T> inline void print(T value, int base) { vout() << format(value, base).rdbuf(); }
    void println() { vout() << '\n'; }
    void println(double value) { print(value); println(); }
    template <typename T> inline void println(T value) { print(value); println(); }
    template <typename T> inline void println(T value, int base) { print(value, base); println(); }
    size_t write(uint8_t* buf, int size);
    inline void flush() { vout().flush(); }
	inline operator bool() { return internal_comm_en ? vehicle_out : fout; }

    inline void enableInternalCommunication() { internal_comm_en = true; }
private:
    int fId;
    bool internal_comm_en;
    
    union {
    struct {
        std::ostream* fout;
        std::string filepath;
    };
    struct {
        std::stringstream* vehicle_out;
        std::stringstream* vehicle_in;
    };
    };

    inline std::ostream& vout() {
        std::ostream* out = internal_comm_en ? vehicle_out : fout;
        if (!out) throw SerialException("Serial %d not enabled", fId);
        return *out;
    }

    template <typename T> inline std::stringstream format(T value, int base) {
        std::stringstream ss;
        if (base == 2) ss << std::bitset<sizeof(value) << 3>(value);
        else if (base == 8) ss << std::oct << value;
        else if (base == 16) ss << std::hex << value;
        else ss << value;
        return ss;
    }
};

extern MockSerial Serial;

#ifndef HYTECH_ARDUINO_UNO
#define Serial1 Serial
extern MockSerial Serial2;
#endif