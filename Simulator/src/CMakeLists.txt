set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BUILD_SHARED_LIBS ON)

add_library(ADAFRUIT_SENSOR INTERFACE)
target_include_directories(ADAFRUIT_SENSOR INTERFACE ${LIB_DIR}/Adafruit_Sensor)

add_library(ADC_SPI SHARED ${LIB_DIR}/ADC_SPI/ADC_SPI.cpp)
target_include_directories(ADC_SPI PUBLIC ${LIB_DIR}/ADC_SPI ${SIMULATION_INC_DIR}/MockADC)
target_link_libraries(ADC_SPI PUBLIC SPI)

add_library(ADXL SHARED ${LIB_DIR}/Adafruit_ADXL345/Adafruit_ADXL345_U.cpp)
target_include_directories(ADXL PUBLIC ${LIB_DIR}/Adafruit_ADXL345 ${SIMULATION_INC_DIR}/MockADXL)
target_link_libraries(ADXL PUBLIC ADAFRUIT_SENSOR CORE WIRE)

add_library(BOARDDEF INTERFACE)
target_include_directories(BOARDDEF INTERFACE ${SIMULATION_INC_DIR}/BoardDef)

add_library(EXPANDER SHARED ${LIB_DIR}/MCP23S17/MCP23S17.cpp)
target_include_directories(EXPANDER PUBLIC ${LIB_DIR}/MCP23S17 ${SIMULATION_INC_DIR}/MockMCP)
target_link_libraries(EXPANDER PUBLIC SPI)

add_library(HYTECH_CAN INTERFACE)
target_include_directories(HYTECH_CAN INTERFACE ${LIB_DIR}/HyTech_CAN)

add_library(LT_SPI SHARED ${LIB_DIR}/LT_SPI/LT_SPI.cpp)
target_include_directories(LT_SPI PUBLIC ${LIB_DIR}/LT_SPI ${LIB_DIR}/Linduino)
target_link_libraries(LT_SPI PUBLIC SPI)

add_library(LTC68042 SHARED ${LIB_DIR}/LTC68042/LTC68042.cpp)
target_include_directories(PUBLIC ${LIB_DIR}/LTC68042 ${LIB_DIR}/Linduino ${SIMULATION_INC_DIR}/MockLTC)
target_link_libraries(LTC68042 PUBLIC LT_SPI)

add_library(METRO SHARED ${LIB_DIR}/Metro/Metro.cpp)
target_include_directories(METRO PUBLIC ${LIB_DIR}/Metro)
target_link_libraries(METRO PUBLIC CORE)

add_library(VEHICLE_UTILITIES INTERFACE)
target_include_directories(VEHICLE_UTILITIES INTERFACE ${LIB_DIR}/VehicleUtilities)
target_link_libraries(VEHICLE_UTILITIES INTERFACE CORE METRO)

add_library(XBTOOLS SHARED ${LIB_DIR}/XBTools/XBTools.c)
target_include_directories(METRO PUBLIC ${LIB_DIR}/XBTools)

add_subdirectory(Adafruit_GPS)
add_subdirectory(core)
add_subdirectory(gtest)
add_subdirectory(HyTech_FlexCAN)
add_subdirectory(MockCAN)
add_subdirectory(mcp_can)
add_subdirectory(SD)
add_subdirectory(SPI)
add_subdirectory(Time)
add_subdirectory(Wire)