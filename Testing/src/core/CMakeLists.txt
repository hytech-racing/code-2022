set(CORE_INC_DIR ${SIMULATION_INCLUDE_DIR}/core)
set(CORE_SRC Arduino.cpp
             HTException.cpp
             Interrupts.cpp
             MockPin.cpp
             Simulator.cpp
             Serial.cpp)

add_library(CORE SHARED ${CORE_SRC})
target_include_directories(CORE PUBLIC "${CORE_INC_DIR}"
                                       "${CMAKE_CURRENT_LIST_DIR}")   