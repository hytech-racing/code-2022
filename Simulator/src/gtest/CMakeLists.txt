set(GTEST_INC_DIR ${INC}/gtest)
set(GTEST_SRC gtest_main.cc
              gtest-all.cc)

add_library(GTEST SHARED ${GTEST_SRC})
target_include_directories(GTEST PUBLIC "${GTEST_INC_DIR}"
                                       "${CMAKE_CURRENT_LIST_DIR}")   
target_link_libraries(GTEST PRIVATE Threads::Threads)