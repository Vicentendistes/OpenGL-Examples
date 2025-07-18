function(add_example name)
  file(GLOB SOURCES ${name}/*.cpp ${name}/*.h)
  if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
  endif(COMMAND cmake_policy)
  add_executable(${name} ${SOURCES})
  target_link_libraries(${name} PRIVATE OpenGLLib GameOfLifeLib)
  target_compile_definitions(${name} PRIVATE SHADER_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${name}/shaders/" RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/")
endfunction()



add_example(grid)
find_package(OpenCL)
if(OpenCL_FOUND)
message("OpenCL found")
  add_example(grid_opencl)
  target_link_libraries(grid_opencl PRIVATE OpenCL::OpenCL)
  target_compile_definitions(grid_opencl
    PRIVATE
      CL_TARGET_OPENCL_VERSION=300
  )
  set(CL_VERSION ${OpenCL_VERSION_MAJOR}${OpenCL_VERSION_MINOR}0)
endif()
