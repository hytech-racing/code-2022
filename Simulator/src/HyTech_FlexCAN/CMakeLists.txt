add_library					( FLEXCAN SHARED HyTech_FlexCAN.cpp )
target_compile_options		( FLEXCAN PUBLIC -DLIBRARY_INCLUDE="${LIB}/HyTech_FlexCAN/IFCT.h" )
target_include_directories	( FLEXCAN PUBLIC ${INC}/HyTech_FlexCAN )
target_link_libraries		( FLEXCAN PUBLIC BOARDDEF CORE CAN )