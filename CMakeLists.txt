cmake_minimum_required(VERSION 4.1.2)
project(TheLimitlessBankSimulator C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(bin)
include_directories(data)
include_directories(include)
include_directories(src)

add_executable(TheLimitlessBankSimulator
        include/db_manager.h
        src/db_manager.c
        src/db_manager.c
        include/structures.h
        src/db_manager.c
        src/main.c
        src/server.c
        include/server.h
        src/client.c
        include/client.h
        src/communication.c
        src/communication.h)
