cmake_minimum_required(VERSION 3.26)
project(ch3 VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
execute_process(COMMAND ln -sf compile_commands.json "${CMAKE_CURRENT_LIST_DIR}")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(SOURCE_FILES
   src/main.c
)

add_executable(app 
   ${SOURCE_FILES}
)

set_property(TARGET app PROPERTY C_STANDARD 99)

# -lm : Target math library for C
target_link_libraries(app m)
