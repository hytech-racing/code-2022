#pragma once

#include <exception>
#include <string>

struct CustomException : public std::exception {
    CustomException();
    CustomException(const char* msg);
    const char* what() const throw();
    protected: char msg [128];
};

struct DoublePinModeException : public CustomException {
    DoublePinModeException(int pin, bool currentMode, bool newMode);
};
struct FileNotOpenException : public CustomException {
    FileNotOpenException(int pin, std::string filepath);
};
struct InterruptNotEnabledException : public CustomException {
    InterruptNotEnabledException(int interruptId);
};
struct InvalidPinConfigurationException : public CustomException {
    InvalidPinConfigurationException(int pin, bool expectedMode, int actualMode);
};
struct InvalidHeaderException : public CustomException {
    InvalidHeaderException(std::string expected, std::string actual);
};