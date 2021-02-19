if(WIN32)
    message( FATAL_ERROR "Error: Windows build not implemented yet")
endif(WIN32)

cmake_minimum_required(VERSION 3.10)
project(simulation)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string(TOUPPER "${arduino}" arduino)
add_compile_options(-DHYTECH_ARDUINO_${arduino})
add_compile_options(-DARDUINO=101)

set(INC ${PROJECT_SOURCE_DIR}/include)
set(SRC ${PROJECT_SOURCE_DIR}/src)
set(VEHICLE_DIR ${PROJECT_SOURCE_DIR}/../Vehicle)
set(LIB ${PROJECT_SOURCE_DIR}/../Libraries)

### ADD NEW TEST FOLDERS HERE ###

add_subdirectory(src)
add_subdirectory(test)