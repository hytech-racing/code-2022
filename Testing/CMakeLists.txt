if(WIN32)
    message( FATAL_ERROR "Error: Windows build not implemented yet")
endif(WIN32)

cmake_minimum_required(VERSION 3.10)
project(simulation)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_SRANDARD_REQUIRED ON)

# find_package(GTest 1.6 REQUIRED)
set(SIMULATION_INC_DIR ${PROJECT_SOURCE_DIR}/include)
set(SIMULATION_SRC_DIR ${PROJECT_SOURCE_DIR}/src)

add_subdirectory(src)