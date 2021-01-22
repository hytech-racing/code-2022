######################## Fill in the following values ########################

set(BINARY        mcu_test   )  # Name of testing output executable
set(VEHICLE_CODE  Main_Control_Unit  )  # Path to folder contianing .ino file (relative to '/Vehicle')
set(TEST_SOURCES  "")  # Space-separated list of test source files
set(CODE_DEPS     ADC_SPI EXPANDER FLEXCAN HYTECH_CAN METRO VEHICLE_UTILITIES)  # Space-separated list of library dependencies
                              # Always included: GTEST, CORE

################### Do not change anything past this point ###################

set(VEHICLE_CODE_DIR ${VEHICLE_DIR}/${VEHICLE_CODE})
set(VEHICLE_CODE ${VEHICLE_CODE_DIR}/${VEHICLE_CODE}.ino)
set_source_files_properties(${VEHICLE_CODE} PROPERTIES LANGUAGE CXX)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_definitions("-x c++")
endif()

add_executable(${BINARY} ${VEHICLE_CODE} ${TEST_SOURCES})
target_link_libraries(${BINARY} PUBLIC GTEST CORE ${CODE_DEPS})
target_include_directories(${BINARY} PUBLIC ${VEHICLE_CODE_DIR})

################################ End template ################################