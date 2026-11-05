cmake_minimum_required(VERSION 3.10)
project(can_log_decoder CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(decoder decoder.cpp)
add_executable(candecode main.cpp)
target_link_libraries(candecode decoder)
add_executable(decoder_test test.cpp)
target_link_libraries(decoder_test decoder)

enable_testing()
add_test(NAME decoder_test COMMAND decoder_test)
