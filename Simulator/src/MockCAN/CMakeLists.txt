add_library					( CAN SHARED MockCAN.cpp )
target_include_directories	( CAN PUBLIC ${INC}/MockCAN ${LIB}/HyTech_FlexCAN )
target_link_libraries		( CAN PUBLIC CORE )