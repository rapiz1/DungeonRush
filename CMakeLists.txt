cmake_minimum_required (VERSION 2.8)
project (DungeonRush)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

aux_source_directory(src SNAKE_SOURCE)
add_executable(snake ${SNAKE_SOURCE})
target_link_libraries(snake SDL2::Main SDL2::Image SDL2::TTF SDL2::Mixer m)
