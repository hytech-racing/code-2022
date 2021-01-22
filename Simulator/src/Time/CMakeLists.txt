add_library(TIME SHARED TimeLib.cpp)
target_include_directories(TIME PUBLIC ${SIMULATION_INC_DIR}/Time)