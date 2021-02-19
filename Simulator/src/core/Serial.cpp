#include <iostream>

#include "Arduino.h"
#include "HTException.h"
#include "Serial.h"

void SerialListener::begin() { serial->attachListener(this); serial->enableInternalCommunication(); }
void SerialListener::teardown() { serial->removeListener(this); }

void HardwareSerial::begin(unsigned int baudRate, uint8_t outmode) {
    if (*this)
        throw SerialException("Tried to begin Serial %d twice", fId);
    if (internal_comm_en) {
        vehicle_in = new std::stringstream;
        vehicle_out = new std::stringstream;
    }
    else {
        fin = infilePath.size() ? new std::ifstream(infilePath, std::ios_base::in) : &std::cin;
        fout = outfilePath.size() ? new std::ofstream(outfilePath, (std::ios_base::openmode) outmode) : &std::cout;

        if (fin->fail()) throw HTException("File Exception", "Unable to open file %s", infilePath.c_str());
        if (fout->fail()) throw HTException("File Exception", "Unable to open file %s", outfilePath.c_str());
    }
}

void HardwareSerial::teardown() {
    if (internal_comm_en) {
        delete vehicle_in;
        delete vehicle_out;
    }
    else {
        if (infilePath.size()) {
            ((std::ifstream*)(fin))->close();
            delete fin;
        }
        if (outfilePath.size()) {
            ((std::ofstream*)(fout))->close();
            delete fout;
        }
    }

    listeners.clear();
    infilePath.clear();
    outfilePath.clear();
    internal_comm_en = false;
    fin = nullptr;
    fout = nullptr;
}

size_t HardwareSerial::write(uint8_t* buf, int size) {
    std::ostream& os = ostream();
    for (int i = 0; i < size; ++i)
        os << std::hex << buf[i];
    return size;
}

HardwareSerial Serial(0);

#ifndef HYTECH_ARDUINO_UNO
HardwareSerial Serial1(1);
HardwareSerial Serial2(2);
#endif