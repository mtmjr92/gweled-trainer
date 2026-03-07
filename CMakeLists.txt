cmake_minimum_required(VERSION 3.10)
project(gweled-hack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gweled-hack
    src/main.cpp
    src/config.cpp
    src/process_memory.cpp
    src/gweled_hack.cpp
)

target_include_directories(gweled-hack PRIVATE src)
