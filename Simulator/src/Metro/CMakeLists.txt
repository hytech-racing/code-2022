set(METRO_INC_DIR ${PROJECT_SOURCE_DIR}/../Libraries/Metro)
set(METRO_SIM_INC_DIR ${SIMULATION_INC_DIR}/Metro)
set(METRO_SRC_DIR ${PROJECT_SOURCE_DIR}/Metro)
set(METRO_SRC Metro.cpp)

add_library(METRO SHARED ${METRO_SRC})
target_include_directories(METRO PUBLIC "${METRO_INC_DIR}"
                                        "${METRO_SIM_INC_DIR}"
                                       "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(METRO PUBLIC CORE)