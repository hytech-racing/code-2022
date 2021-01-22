add_library(MCP_CAN SHARED mcp_can.cpp)
target_include_directories(MCP_CAN PUBLIC ${LIB_DIR}/mcp_can)
target_link_libraries(MCP_CAN PUBLIC CORE CAN SPI)