cmake_minimum_required(VERSION 3.10)
project(gweled-hack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")

add_executable(gweled-hack
    src/main.cpp
    src/config.cpp
    src/process_memory.cpp
    src/gweled_hack.cpp
)

target_include_directories(gweled-hack PRIVATE src)

# Copy config.json next to the binary after build
add_custom_command(TARGET gweled-hack POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config.json
        $<TARGET_FILE_DIR:gweled-hack>/config.json
)
