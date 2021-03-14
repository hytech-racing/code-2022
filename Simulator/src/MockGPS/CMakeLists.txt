add_library					( GPS SHARED MockGPS.cpp )
target_include_directories	( GPS PUBLIC ${INC}/MockGPS )
target_link_libraries		( GPS PUBLIC CORE ADAFRUIT_GPS TIME )