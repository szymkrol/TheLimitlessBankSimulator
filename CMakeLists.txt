cmake_minimum_required(VERSION 4.2)
project(TheLimitlessBankSimulator C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(bin)
include_directories(data)
include_directories(include)
include_directories(src)

add_executable(TheLimitlessBankSimulator
        include/db_manager.h
        src/db_manager.cpp
        src/db_manager.cpp
        include/structures.h
        src/db_manager.cpp)
