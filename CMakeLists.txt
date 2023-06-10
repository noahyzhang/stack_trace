cmake_minimum_required(VERSION 3.25.0)
project(stack_trace)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -Wno-error=unused-parameter -fno-omit-frame-pointer -std=c++11")

include_directories(
    include/
)

file(GLOB TEST_STACK_TRACE
    test/test_stack_trace.cpp 
)

add_executable(test_stack_trace ${TEST_STACK_TRACE})

target_link_libraries(test_stack_trace
    bfd
    dl
)
