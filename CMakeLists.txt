cmake_minimum_required(VERSION 3.19)
project(SGM)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

set(CMAKE_SYSTEM_NAME Linux)

add_definitions(-DSGM51620=1)
add_executable(sgm51242 sgm51242.c sgm51620.c)