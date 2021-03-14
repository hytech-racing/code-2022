add_library					( SPI SHARED SPI.cpp SPIListener.cpp SwapRegister.cpp )
target_include_directories	( SPI PUBLIC ${INC}/SPI )
target_link_libraries		( SPI PUBLIC CORE )