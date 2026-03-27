cmake_minimum_required(VERSION 3.16)
project(GalaxyStorm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# All game source files
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/player.cpp
    src/enemy.cpp
    src/bullet.cpp
    src/boss.cpp
    src/collision.cpp
    src/stage.cpp
    src/powerup.cpp
    src/sprites.cpp
    src/hud.cpp
    src/menu.cpp
    src/background.cpp
)

# Headers live in src/ (alongside .cpp files)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Executable — WIN32 suppresses the console window on Windows
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# Link SDL2 — support both modern SDL2 (imported targets via SDL2Config.cmake,
# SDL2 >= 2.0.12) and legacy installs (plain variables via FindSDL2.cmake).
if(TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
    if(TARGET SDL2::SDL2main)
        target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)
    endif()
else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
endif()

# Copy SDL2 DLL on Windows (only when using modern imported targets)
if(WIN32 AND TARGET SDL2::SDL2)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
