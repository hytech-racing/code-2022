set(BINARY dashboard_test)
set(VEHICLE_CODE ${PROJECT_SOURCE_DIR}/../Vehicle/Dashboard/Dashboard.ino)
# set_source_files_properties(${VEHICLE_CODE} PROPERTIES COMPILE_FLAGS "-x c++")
# set(TEST_SOURCES ${VEHICLE_CODE} testing.cpp)
set(TEST_SOURCES testing.cpp Dashboard.cpp)

add_executable(${BINARY} ${TEST_SOURCES})

target_link_libraries(${BINARY} PUBLIC GTEST CORE CAN_SIMULATOR MCP_CAN METRO)
target_link_libraries(${BINARY} PUBLIC HYTECH_CAN VEHICLE_UTILITIES)
