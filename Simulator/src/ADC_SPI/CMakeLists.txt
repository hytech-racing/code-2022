add_library(ADC_SPI SHARED ADC_SPI.cpp)
target_include_directories(ADC_SPI PUBLIC ${LIB_DIR}/ADC_SPI/)
target_link_libraries(ADC_SPI PUBLIC SPI)