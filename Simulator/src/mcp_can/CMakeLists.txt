set(MCP_CAN_INC_DIR ${PROJECT_SOURCE_DIR}/../Libraries/mcp_can)
set(MCP_CAN_SIM_INC_DIR ${SIMULATION_INC_DIR}/mcp_can)
set(MCP_CAN_SRC mcp_can.cpp)

add_library(MCP_CAN SHARED ${MCP_CAN_SRC})
target_include_directories(MCP_CAN PUBLIC "${MCP_CAN_INC_DIR}"
                                        "${MCP_CAN_SIM_INC_DIR}"
                                       "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(MCP_CAN PUBLIC CORE CAN_SIMULATOR)