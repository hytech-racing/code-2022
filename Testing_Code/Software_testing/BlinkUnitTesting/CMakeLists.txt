cmake_minimum_required(VERSION 3.8)
project(BlinkUnitTesting)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Blink.cpp Blink.h)
add_executable(BlinkUnitTesting ${SOURCE_FILES})