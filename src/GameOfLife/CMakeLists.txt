add_definitions(-DCL_TARGET_OPENCL_VERSION=200)

add_library(GameOfLifeLib
  game_of_life_serial.cpp
  game_of_life_opencl.cpp
)

target_compile_definitions(GameOfLifeLib
  PRIVATE
    KERNEL_FILE="${CMAKE_CURRENT_SOURCE_DIR}/kernel.cl"
)

target_include_directories(GameOfLifeLib
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include/GameOfLife
)

find_package(OpenCL)
find_package(OpenGL REQUIRED)
target_link_libraries(GameOfLifeLib
  PUBLIC
    OpenCL::OpenCL             # la implementación de OpenCL
    glfw                       # la librería GLFW
    OpenGL::GL                 # la librería OpenGL
)