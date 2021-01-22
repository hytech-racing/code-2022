set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BUILD_SHARED_LIBS ON)

add_library(ADAFRUIT_SENSOR INTERFACE)
target_include_directories(ADAFRUIT_SENSOR INTERFACE ${LIB_DIR}/Adafruit_Sensor)

add_library(ADXL INTERFACE)
target_include_directories(ADXL INTERFACE ${LIB_DIR}/Adafruit_ADXL345)
target_link_libraries(ADXL INTERFACE WIRE)

add_library(BOARDDEF INTERFACE)
target_include_directories(BOARDDEF INTERFACE ${SIMULATION_INC_DIR}/BoardDef)

add_library(GPS INTERFACE)
target_include_directories(GPS INTERFACE ${LIB_DIR}/Adafruit_GPS-master)
target_compile_options(GPS INTERFACE -DHardwareSerial=MockSerial)
target_link_libraries(GPS INTERFACE CORE)

add_library(HYTECH_CAN INTERFACE)
target_include_directories(HYTECH_CAN INTERFACE ${LIB_DIR}/HyTech_CAN)

add_library(METRO SHARED ${LIB_DIR}/Metro/Metro.cpp)
target_include_directories(METRO PUBLIC ${LIB_DIR}/Metro)
target_link_libraries(METRO PUBLIC CORE)

add_library(SPI INTERFACE)
target_include_directories(SPI INTERFACE ${SIMULATION_INC_DIR}/SPI)
target_link_libraries(SPI INTERFACE CORE)

add_library(VEHICLE_UTILITIES INTERFACE)
target_include_directories(VEHICLE_UTILITIES INTERFACE ${LIB_DIR}/VehicleUtilities)
target_link_libraries(VEHICLE_UTILITIES INTERFACE CORE METRO)

add_library(WIRE INTERFACE)
target_include_directories(WIRE INTERFACE ${SIMULATION_INC_DIR}/Wire)

add_library(XBTOOLS SHARED ${LIB_DIR}/XBTools/XBTools.c)
target_include_directories(METRO PUBLIC ${LIB_DIR}/XBTools)

add_subdirectory(ADC_SPI)
add_subdirectory(core)
add_subdirectory(gtest)
add_subdirectory(HyTech_FlexCAN)
add_subdirectory(LT_SPI)
add_subdirectory(LTC68042)
add_subdirectory(MCP23S17)
add_subdirectory(MockCAN)
add_subdirectory(mcp_can)
add_subdirectory(SD)
add_subdirectory(Time)