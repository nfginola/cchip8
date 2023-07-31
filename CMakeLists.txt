cmake_minimum_required(VERSION 3.26)
project(ch3 VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
execute_process(COMMAND ln -sf compile_commands.json "${CMAKE_CURRENT_LIST_DIR}")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(SOURCE_FILES
   src/main.c
   src/adr_stack.c
   src/sdl_helper.c
   src/chip8.c
   src/viz_bits.c
   src/utils.c
   src/viz_internals.c
)

add_executable(app 
   ${SOURCE_FILES}
)

set_property(TARGET app PROPERTY C_STANDARD 99)

# -lm : Target math library for C
target_link_libraries(app m)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(app ${SDL2_LIBRARIES})

target_compile_definitions(app PRIVATE CHIP8)
#target_compile_definitions(app PRIVATE SCHIP)
#target_compile_definitions(app PRIVATE XOCHIP)

