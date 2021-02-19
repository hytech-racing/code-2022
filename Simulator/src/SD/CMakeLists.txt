add_library					( SD SHARED SD.cpp )
target_include_directories	( SD PUBLIC ${INC}/SD )
target_link_libraries		( SD PUBLIC CORE )