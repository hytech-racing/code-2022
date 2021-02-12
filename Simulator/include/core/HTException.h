#pragma once

#include <cstring>
#include <exception>
#include <string>
#include <stdarg.h>

#include "MockPin.h"

struct HTException : public std::exception {
    HTException(const char* const type, const char* const message, ...) {
        strcpy(this->type, type);
        va_list args;
        va_start(args, message);
        vsprintf(msg, message, args);
        va_end(args);
    }

    inline const char* what() const throw() {
        printf("\n\n%s\n%s\n\n", type, msg);
        return type;
    }

protected:
    char type [128];
    char msg [128];
};

#define CANException(...) HTException("CAN Exception", __VA_ARGS__)
#define SPIException(...) HTException("SPI Exception", __VA_ARGS__)
#define SetupException(...) HTException("Setup Exception", __VA_ARGS__)
#define FileNotOpenException(...) HTException("File Not Open Exception", "Pin %d was unable to open file %s", __VA_ARGS__)
#define I2CException(...) HTException("I2C Listener Error", __VA_ARGS__)
#define MCPException(type, channel) HTException("MCP Exception", type " ID must be in range 0-15 inclusive. Received %d.", channel)
#define LTCException(...) HTException("LTC Exception", __VA_ARGS__)

struct DoublePinModeException : public HTException {
    DoublePinModeException(int pin, bool currentMode, bool newMode) : HTException(
        "Double Pin Mode Exception",
        "Pin mode set twice -- Pin %d was %s was set to %s\n",
        pin, &decodePinMode(currentMode)[0], &decodePinMode(newMode)[0]
    ) {}
};

struct InvalidPinConfigurationException : public HTException {
    InvalidPinConfigurationException(int pin, bool expectedMode, int actualMode) : HTException(
        "Invalid Pin Configuration Exception",
        "Invalid pin configuration -- Pin %d expected %s but got %s\n",
        pin, &decodePinMode(expectedMode)[0], &decodePinMode(actualMode)[0]
    ) {}
};