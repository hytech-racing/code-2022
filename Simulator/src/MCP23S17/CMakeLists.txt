add_library(EXPANDER SHARED MCP23S17.cpp)
target_include_directories(EXPANDER PUBLIC ${LIB_DIR}/MCP23S17 ${SIMULATION_INC_DIR}/MockExpander)
target_link_libraries(EXPANDER PUBLIC SPI)