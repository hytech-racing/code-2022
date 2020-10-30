set(CAN_SIM_INC_DIR ${SIMULATION_INC_DIR}/CAN_sim)
set(HYTECH_FLEX_CAN_INC_DIR ${PROJECT_SOURCE_DIR}/../Libraries/HyTech_FlexCAN)
set(CAN_SIM_SRC CAN_sim.cpp)

add_library(CAN_SIM SHARED ${CAN_SIM_SRC})
target_include_directories(CAN_SIM PUBLIC "${CAN_SIM_INC_DIR}"
                                          "${HYTECH_FLEX_SIM_INC_DIR}"
                                          "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(CAN_SIM PUBLIC CORE)