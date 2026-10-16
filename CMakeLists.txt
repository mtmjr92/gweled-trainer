cmake_minimum_required(VERSION 3.14)
project(gweled-trainer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")

include(FetchContent)
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v5.0.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(ftxui)

find_package(Threads REQUIRED)

add_executable(gweled-trainer
    src/main.cpp
    src/config.cpp
    src/process_memory.cpp
    src/trainer.cpp
    src/feature.cpp
    src/tui.cpp
)

target_include_directories(gweled-trainer PRIVATE src)
target_link_libraries(gweled-trainer PRIVATE
    ftxui::component
    ftxui::dom
    ftxui::screen
    Threads::Threads
)

# Copy config.json next to the binary after build
add_custom_command(TARGET gweled-trainer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config.json
        $<TARGET_FILE_DIR:gweled-trainer>/config.json
)
