add_library(WIRE SHARED Wire.cpp)
target_include_directories(WIRE PUBLIC ${SIMULATION_INC_DIR}/Wire)
target_link_libraries(WIRE PUBLIC CORE)

