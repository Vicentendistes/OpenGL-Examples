file(GLOB SOURCES *.cpp *.h)
find_package(OpenGL REQUIRED)
add_library(OpenGLLib ${SOURCES})
target_link_libraries(OpenGLLib PUBLIC glfw glad glm stb imgui OpenGL::GL)
target_link_libraries(imgui PRIVATE glfw)
target_include_directories(OpenGLLib PUBLIC "${CMAKE_SOURCE_DIR}/include")


find_package(OpenCL)
if(OpenCL_FOUND)
message("OpenCL found")
  target_link_libraries(OpenGLLib PUBLIC OpenCL::OpenCL)
  target_include_directories(OpenGLLib
  PUBLIC
    ${OpenCL_INCLUDE_DIRS}
  )
  # set(CL_VERSION ${OpenCL_VERSION_MAJOR}${OpenCL_VERSION_MINOR}0)
  # target_compile_definitions(opencl PRIVATE TEST=${CL_VERSION})
endif()