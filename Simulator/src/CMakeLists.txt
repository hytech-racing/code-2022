set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BUILD_SHARED_LIBS ON)

add_library					( BOARDDEF INTERFACE )
target_include_directories	( BOARDDEF INTERFACE ${INC}/BoardDef )

add_subdirectory(Vehicle)

add_subdirectory(core)
add_subdirectory(gtest)
add_subdirectory(HyTech_FlexCAN)
add_subdirectory(mcp_can)
add_subdirectory(MockCAN)
add_subdirectory(MockGPS)
add_subdirectory(MockLTC)
add_subdirectory(MockMCP)
add_subdirectory(SD)
add_subdirectory(SPI)
add_subdirectory(Time)
add_subdirectory(Wire)

add_library					( ACCEL INTERFACE )
target_include_directories	( ACCEL INTERFACE ${INC}/MockADXL )
target_link_libraries		( ACCEL INTERFACE ADXL345 )

add_library					( ADC INTERFACE )
target_include_directories	( ADC INTERFACE ${INC}/MockADC )
target_link_libraries		( ADC INTERFACE ADC_SPI )