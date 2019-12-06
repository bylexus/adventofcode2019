cmake_minimum_required(VERSION 3.0.0)
project(adventofcode2019 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(day_01 src/day_01.cpp)
add_executable(day_02 src/day_02.cpp src/common.cpp)
add_executable(day_03 src/day_03.cpp src/common.cpp)
add_executable(day_04 src/day_04.cpp src/common.cpp)
add_executable(day_05 src/day_05.cpp src/common.cpp)
add_executable(day_06 src/day_06.cpp src/common.cpp)


set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

