add_library(LT_SPI SHARED LT_SPI.cpp)
target_include_directories(LT_SPI PUBLIC ${LIB_DIR}/LT_SPI ${LIB_DIR}/Linduino)
target_link_libraries(LT_SPI PUBLIC SPI)
