add_library(GPS SHARED Adafruit_GPS.cpp)
target_compile_options(GPS PUBLIC -DHardwareSerial=MockSerial)
target_include_directories(GPS PUBLIC ${LIB_DIR}/Adafruit_GPS-master ${SIMULATION_INC_DIR}/MockGPS)
target_link_libraries(GPS PUBLIC CORE)

