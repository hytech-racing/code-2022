#pragma once

#include <bitset>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "BoardDef.h"
#include "HTException.h"
#include "Listener.h"
#include "MockPin.h"
#include "Print.h"

#define BIN 2
#define OCT 8
#define HEX 16

class HardwareSerial;

class SerialListener : public Listener {
public:
	SerialListener(HardwareSerial* serial) : serial(serial) {}
	virtual void begin();
	virtual void teardown();
	virtual void processString(const char* str) = 0;

	virtual void process(uint8_t pinNo, unsigned value) {
		throw SerialException("Do not use process for SerialListener. Use processString instead");
	}
protected:
	HardwareSerial* serial;
};

class HardwareSerial {
public:
    HardwareSerial(int id) : fId(id), internal_comm_en(false), fin(nullptr), fout(nullptr) {}
    void begin(unsigned int baudRate, uint8_t outmode = std::ios::ios_base::out);
    inline void end() { teardown(); }
    void teardown();
    inline void setInputPath(std::string filepath) { infilePath = filepath; }
    inline void setOutputPath(std::string filepath) { outfilePath = filepath; }

    bool available() { return istream().rdbuf()->in_avail(); }
    char read() { return istream().get(); }

    size_t write(uint8_t* buf, int size);
    inline void flush() { ostream().flush(); }
	inline operator bool() { return internal_comm_en ? vehicle_out : fout; }

    inline void enableInternalCommunication() { internal_comm_en = true; }
    inline void attachListener(SerialListener* listener) { listeners.push_back(listener); }
    inline void removeListener(SerialListener* listener) { listeners.remove(listener); }
    inline void sim_println(char* str) { if (internal_comm_en) *vehicle_in << str << '\n'; else throw SerialException("Internal communication not enabled"); }

    void println() { ostream() << '\n'; triggerEvents(); }

    template<typename T> inline void print(T value)     { ostream() << value; triggerEvents(); }
    template<typename T> inline void println(T value)   { ostream() << value << '\n'; triggerEvents(); }

    template<typename T> inline void print(T value, int formatter)      { print_nolisten(value, formatter); triggerEvents(); }
    template<typename T> inline void println(T value, int formatter)    { print_nolisten(value, formatter) << '\n'; triggerEvents();  }

private:

#define printfloat(T)   inline std::ostream& print_nolisten(T value, int precision) { return ostream() << std::setprecision(precision) << value; }
#define printint(T)     inline std::ostream& print_nolisten(T value, int base) { return printformatted(value, base); }

    printfloat(float)
    printfloat(double)
    printfloat(long double)

    printint(char)
    printint(int)
    printint(short)
    printint(long)
    printint(long long)

    printint(unsigned char)
    printint(unsigned)
    printint(unsigned short)
    printint(unsigned long)
    printint(unsigned long long)

#undef printfloat
#undef printint

    int fId;
    bool internal_comm_en;
    std::string infilePath, outfilePath;

    union { 
    struct { std::istream* fin; std::ostream* fout; };
    struct { std::stringstream* vehicle_in, *vehicle_out; };
    };

    inline std::ostream& ostream() {
        std::ostream* out = internal_comm_en ? vehicle_out : fout;
        if (!out) throw SerialException("Serial %d not enabled", fId);
        return *out;
    }

    inline std::istream& istream() {
        std::istream* in = internal_comm_en ? vehicle_in : fin;
        if (!in) throw SerialException("Serial %d not enabled", fId);
        return *in;
    }

    template <typename T> inline std::ostream& printformatted(T value, int base) {
        if (base == BIN) return ostream() << std::bitset<sizeof(value) << 3>(value);
        if (base == OCT) return ostream() << std::oct << value;
        if (base == HEX) return ostream() << std::hex << value;
        return ostream() << value;
    }

    std::list<SerialListener*> listeners;
    inline void triggerEvents() {
        if (internal_comm_en && listeners.size()) {
            std::string str; getline(*vehicle_out, str);
            for (SerialListener* listener : listeners) listener->processString(str.c_str());
        }
    }
        
};

extern HardwareSerial Serial;

#ifndef HYTECH_ARDUINO_UNO
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
#endif