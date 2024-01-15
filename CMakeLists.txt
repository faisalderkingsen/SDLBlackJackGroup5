cmake_minimum_required(VERSION 3.10)

project(MeinSDLProjekt)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

file(GLOB sources *.c)

add_executable(mein_code ${sources})

target_link_libraries(mein_code ${SDL2_LIBRARIES})
