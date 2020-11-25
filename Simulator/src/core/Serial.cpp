#include <iostream>

#include "Arduino.h"
#include "HTException.h"
#include "Serial.h"

MockSerial::MockSerial(int id) { fId = id; }

MockSerial::~MockSerial() {
    if (fFilepath.size()) {
        ((std::ofstream*)(fos))->close();
        delete fos;
    }
    fos = nullptr;
}

void MockSerial::setOutputPath(std::string filepath) { fFilepath = filepath; }

void MockSerial::begin(unsigned int baudRate) {
    if (fos)
        throw DoublePinModeException(fId, OUTPUT, OUTPUT);
    if (fFilepath.size()) {
        std::ofstream* tmpFos = new std::ofstream(fFilepath, std::ios::ios_base::out);
        if (tmpFos->fail())
            throw FileNotOpenException(fId, fFilepath);
        fos = tmpFos;
    }
    else fos = &std::cout;
}

void MockSerial::end() { this->~MockSerial(); }

void MockSerial::write(uint8_t* buf, int size) {
    validate();
    for (int i = 0; i < size; ++i)
        *fos << std::hex << buf[i];
}

MockSerial Serial(-1);
MockSerial Serial2(-2);