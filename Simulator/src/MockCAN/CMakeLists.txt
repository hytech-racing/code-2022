add_library(CAN SHARED MockCAN.cpp)
target_include_directories(CAN PUBLIC ${SIMULATION_INC_DIR}/MockCAN ${LIB_DIR}/HyTech_FlexCAN)
target_link_libraries(CAN PUBLIC CORE)