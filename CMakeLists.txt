cmake_minimum_required(VERSION 3.0.0)
project(adventofcode2019 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/opt/local/include)
link_directories(/opt/local/lib)

add_compile_definitions(cimg_display=0)
add_library(common src/common.cpp)

add_executable(day_01 src/day_01.cpp)
add_executable(day_02 src/day_02.cpp)
add_executable(day_03 src/day_03.cpp)
add_executable(day_04 src/day_04.cpp)
add_executable(day_05 src/day_05.cpp)
add_executable(day_06 src/day_06.cpp)
add_executable(day_07 src/day_07.cpp)
add_executable(day_08 src/day_08.cpp)
add_executable(day_09 src/day_09.cpp)
add_executable(day_10 src/day_10.cpp)

target_link_libraries(day_01 PUBLIC common)
target_link_libraries(day_02 PUBLIC common)
target_link_libraries(day_03 PUBLIC common)
target_link_libraries(day_04 PUBLIC common)
target_link_libraries(day_05 PUBLIC common)
target_link_libraries(day_06 PUBLIC common)
target_link_libraries(day_07 PUBLIC common)
target_link_libraries(day_08 PUBLIC common)
target_link_libraries(day_09 PUBLIC common)
target_link_libraries(day_10 PUBLIC common)


set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
