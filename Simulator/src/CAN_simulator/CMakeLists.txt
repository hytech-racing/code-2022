set(CAN_SIMULATOR_INC_DIR ${SIMULATION_INC_DIR}/CAN_simulator)
set(HYTECH_FLEX_CAN_INC_DIR ${PROJECT_SOURCE_DIR}/../Libraries/HyTech_FlexCAN)
set(CAN_SIMULATOR_SRC CAN_simulator.cpp)

add_library(CAN_SIMULATOR SHARED ${CAN_SIMULATOR_SRC})
target_include_directories(CAN_SIMULATOR PUBLIC "${CAN_SIMULATOR_INC_DIR}"
                                          "${HYTECH_FLEX_CAN_INC_DIR}"
                                          "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(CAN_SIMULATOR PUBLIC CORE)