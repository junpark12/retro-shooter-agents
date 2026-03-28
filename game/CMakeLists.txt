cmake_minimum_required(VERSION 3.16)
project(GalaxyStorm VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 and extension libraries
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

# All game source files
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/player.cpp
    src/enemy.cpp
    src/bullet.cpp
    src/bullet_pattern.cpp
    src/boss.cpp
    src/collision.cpp
    src/stage.cpp
    src/powerup.cpp
    src/asset_manager.cpp
    src/audio.cpp
    src/sprites.cpp
    src/hud.cpp
    src/menu.cpp
    src/background.cpp
)

# Headers live in src/ (alongside .cpp files)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Executable — WIN32 suppresses the console window on Windows
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# ── Link SDL2 (modern vs legacy) ──────────────────────────────────────────────
if(TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
    if(TARGET SDL2::SDL2main)
        target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)
    endif()
else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
endif()

# ── Link SDL2_image ───────────────────────────────────────────────────────────
if(TARGET SDL2_image::SDL2_image)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_image::SDL2_image)
elseif(TARGET SDL2::SDL2_image)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2_image)
else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_LIBRARIES})
endif()

# ── Link SDL2_mixer ───────────────────────────────────────────────────────────
if(TARGET SDL2_mixer::SDL2_mixer)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_mixer::SDL2_mixer)
elseif(TARGET SDL2::SDL2_mixer)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2_mixer)
else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_LIBRARIES})
endif()

# ── Link SDL2_ttf ─────────────────────────────────────────────────────────────
if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)
elseif(TARGET SDL2::SDL2_ttf)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2_ttf)
else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_TTF_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
endif()

# ── Copy DLLs on Windows (modern imported targets only) ───────────────────────
if(WIN32)
    foreach(_tgt SDL2::SDL2 SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf)
        if(TARGET ${_tgt})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${_tgt}>
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    endforeach()
endif()

# ── Copy assets to build directory ────────────────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
