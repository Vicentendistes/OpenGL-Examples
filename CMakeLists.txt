cmake_minimum_required(VERSION 3.13)

project(OpenGLLib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(extern)
add_subdirectory(src/OpenGLLib)
add_subdirectory(src/GameOfLife)

if(EXAMPLES)
  message("Adding examples")
  add_subdirectory(examples)
endif()
