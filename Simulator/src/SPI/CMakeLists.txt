add_library(SPI SHARED SPI.cpp)
target_include_directories(SPI PUBLIC ${SIMULATION_INC_DIR}/SPI)
target_link_libraries(SPI PUBLIC CORE)