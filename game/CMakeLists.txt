cmake_minimum_required(VERSION 3.16)
project(GalaxyStorm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# Source files (에이전트가 구현하면서 추가)
set(SOURCES
    src/main.cpp
)

# Header directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# Link SDL2
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)

# Copy SDL2 DLL on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
