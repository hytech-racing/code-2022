add_library					( LTC SHARED MockLTC.cpp )
target_include_directories	( LTC PUBLIC ${INC}/MockLTC )
target_link_libraries		( LTC PUBLIC LTC68042 )