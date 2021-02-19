add_library                 ( CORE SHARED Interrupts.cpp MockPin.cpp Simulator.cpp Serial.cpp )
target_include_directories  ( CORE PUBLIC ${INC}/core )
target_link_libraries       ( CORE PUBLIC BOARDDEF )