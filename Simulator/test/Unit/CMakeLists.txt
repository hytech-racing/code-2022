set(BINARY unit_tests)
add_executable(unit_tests test_ADC.cpp test_ADXL.cpp test_GPS.cpp test_mcp.cpp test_ltc.cpp)
target_link_libraries(unit_tests PUBLIC GTEST CORE ADC ACCEL GPS EXPANDER LTC)