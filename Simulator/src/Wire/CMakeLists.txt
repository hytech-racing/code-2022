add_library					( WIRE SHARED Wire.cpp )
target_include_directories	( WIRE PUBLIC ${INC}/Wire )
target_link_libraries		( WIRE PUBLIC CORE )

