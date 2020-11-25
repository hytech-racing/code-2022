#include <cstring>

#include "HTException.h"
#include "MockPin.h"

CustomException::CustomException() {}
CustomException::CustomException(const char* msg) { strcpy(this->msg, msg); }
const char* CustomException::what() const throw() { return msg; }

DoublePinModeException::
DoublePinModeException(int pin, bool currentMode, bool newMode) {
    sprintf(&msg[0], "Pin mode set twice -- Pin %d was %s was set to %s\n",
        pin, &decodePinMode(currentMode)[0], &decodePinMode(newMode)[0]);
}

FileNotOpenException::
FileNotOpenException(int pin, std::string filepath) {
    sprintf(&msg[0], "Pin %d was unable to open file %s\n", pin, &filepath[0]);
}

InterruptNotEnabledException::
InterruptNotEnabledException(int interruptId) {
    sprintf(&msg[0], "Cannot attach interrupt vector - interrupt %d not enabled", interruptId);
}

InvalidPinConfigurationException::
InvalidPinConfigurationException(int pin, bool expectedMode, int actualMode) {
    sprintf(&msg[0], "Invalid pin configuration -- Pin %d expected %s but got %s\n",
        pin, &decodePinMode(expectedMode)[0], &decodePinMode(actualMode)[0]);
}

InvalidHeaderException::
InvalidHeaderException(std::string expected, std::string actual) {
    sprintf(&msg[0], "Malformed test file header -- expected %s but got %s\n", &expected[0], &actual[0]);
}