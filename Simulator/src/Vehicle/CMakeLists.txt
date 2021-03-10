### Import Vehicle Libraries ###

# Not Included:
# FlexCAN
# FlexCAN_T4
# genieArduino
# HyTech_FlexCAN
# mcp_can
# Sparkfun_ADXL345
# UserInterface
# xbee-micropython

# Adafruit ADXL345
add_library 				( ADXL345 SHARED ${LIB}/Adafruit_ADXL345/Adafruit_ADXL345_U.cpp )
target_include_directories	( ADXL345 PUBLIC ${LIB}/Adafruit_ADXL345 )
target_link_libraries		( ADXL345 PUBLIC ADAFRUIT_SENSOR CORE WIRE )

# Adafruit GPS
add_library					( ADAFRUIT_GPS SHARED ${LIB}/Adafruit_GPS-master/Adafruit_GPS.cpp )
target_include_directories	( ADAFRUIT_GPS PUBLIC ${LIB}/Adafruit_GPS-master )
target_link_libraries		( ADAFRUIT_GPS PUBLIC CORE )

# Adafruit Sensor
add_library					( ADAFRUIT_SENSOR INTERFACE)
target_include_directories	( ADAFRUIT_SENSOR INTERFACE ${LIB}/Adafruit_Sensor)

# ADC SPI
add_library					( ADC_SPI SHARED ${LIB}/ADC_SPI/ADC_SPI.cpp )
target_include_directories	( ADC_SPI PUBLIC ${LIB}/ADC_SPI )
target_link_libraries		( ADC_SPI PUBLIC SPI )

# HyTech_CAN
add_library					( HYTECH_CAN INTERFACE )
target_include_directories	( HYTECH_CAN INTERFACE ${LIB}/HyTech_CAN )

# Linduino
add_library					( LINDUINO INTERFACE )
target_include_directories	( LINDUINO INTERFACE ${LIB}/Linduino )
target_link_libraries		( LINDUINO INTERFACE SPI )

# LT_SPI
add_library					( LT_SPI SHARED ${LIB}/LT_SPI/LT_SPI.cpp )
target_include_directories	( LT_SPI PUBLIC ${LIB}/LT_SPI )
target_link_libraries		( LT_SPI PUBLIC LINDUINO )

# LTC68042
add_library					( LTC68042 SHARED ${LIB}/LTC68042/LTC68042.cpp )
target_include_directories	( LTC68042 PUBLIC ${LIB}/LTC68042 )
target_link_libraries		( LTC68042 PUBLIC LT_SPI )

# MCP23S17
add_library					( MCP23S17 SHARED ${LIB}/MCP23S17/MCP23S17.cpp )
target_include_directories	( MCP23S17 PUBLIC ${LIB}/MCP23S17 )
target_link_libraries		( MCP23S17 PUBLIC SPI )

# Metro
add_library					( METRO SHARED ${LIB}/Metro/Metro.cpp )
target_include_directories	( METRO PUBLIC ${LIB}/Metro )
target_link_libraries		( METRO PUBLIC CORE )

# VehicleUtilities
add_library					( VEHICLE_UTILITIES INTERFACE )
target_include_directories	( VEHICLE_UTILITIES INTERFACE ${LIB}/VehicleUtilities )
target_link_libraries		( VEHICLE_UTILITIES INTERFACE CORE METRO )

# XBTools
add_library					( XBTOOLS SHARED ${LIB}/XBTools/XBTools.c )
target_include_directories	( METRO PUBLIC ${LIB}/XBTools )