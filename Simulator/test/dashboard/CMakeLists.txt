set(BINARY dashboard_test)

set(VEHICLE_CODE ${VEHICLE_DIR}/Dashboard/Dashboard.ino)
set_source_files_properties(${VEHICLE_CODE} PROPERTIES LANGUAGE CXX)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_definitions("-x c++")
endif()

set(TEST_SOURCES ${VEHICLE_CODE} testing.cpp)

add_executable(${BINARY} ${TEST_SOURCES})

target_link_libraries(${BINARY} PUBLIC GTEST CORE CAN_SIMULATOR MCP_CAN METRO)
target_link_libraries(${BINARY} PUBLIC HYTECH_CAN VEHICLE_UTILITIES)
target_include_directories(${BINARY} PUBLIC ${VEHICLE_DIR}/Dashboard)