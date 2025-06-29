cmake_minimum_required(VERSION 3.13)

project(glcore)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(extern)
add_subdirectory(src)
if(GLCORE_EXAMPLES)
  message("Adding examples")
  add_subdirectory(examples)
endif()
