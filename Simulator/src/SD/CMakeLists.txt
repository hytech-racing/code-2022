add_library(SD SHARED SD.cpp)
target_include_directories(SD PUBLIC ${SIMULATION_INC_DIR}/SD)
target_link_libraries(SD PUBLIC SPI)