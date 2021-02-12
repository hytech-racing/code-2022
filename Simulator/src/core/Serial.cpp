#include <iostream>

#include "Arduino.h"
#include "HTException.h"
#include "Serial.h"

MockSerial::~MockSerial() {
    if (internal_comm_en) {
        delete vehicle_in;
        delete vehicle_out;
        vehicle_in = vehicle_out = nullptr;
    }
    else {
        if (filepath.size()) {
            ((std::ofstream*)(fos))->close();
            delete fos;
            filepath.clear();
        }
        fos = nullptr;
    }
}

void MockSerial::setOutputPath(std::string filepath) { this->filepath = filepath; }

void MockSerial::begin(unsigned int baudRate, uint8_t mode) {
    if (*this)
        throw SerialException("Tried to begin Serial %d twice", fId), 
    if (internal_comm_en) {
        vehicle_in = new std::stringstream;
        vehicle_out = new std::stringstream;
    }
    if (filepath.size()) {
        std::fstream* tmpFos = new std::fstream(fFilepath, (std::ios_base::openmode) mode);
        if (tmpFos->fail())
            throw FileNotOpenException(fId, fFilepath);
        fos = tmpFos;
    }
    else fos = &std::cout;
}

void MockSerial::end() { this->~MockSerial(); }

size_t MockSerial::write(uint8_t* buf, int size) {
    validate();
    for (int i = 0; i < size; ++i)
        *fos << std::hex << buf[i];
    return size;
}

MockSerial Serial(1);

#ifndef HYTECH_ARDUINO_UNO
MockSerial Serial2(2);
#endif