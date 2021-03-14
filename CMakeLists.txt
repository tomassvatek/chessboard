cmake_minimum_required(VERSION 3.9)
project(pdp C)

set(CMAKE_C_STANDARD 99)

find_package(OpenMP)
add_executable(pdp main.c)
if(OpenMP_CXX_FOUND)
    target_link_libraries(pdp PUBLIC OpenMP::OpenMP_CXX)
endif()