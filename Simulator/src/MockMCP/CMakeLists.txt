add_library					( EXPANDER SHARED MockMCP.cpp )
target_include_directories	( EXPANDER PUBLIC ${INC}/MockMCP )
target_link_libraries		( EXPANDER PUBLIC MCP23S17 )