cmake_minimum_required(VERSION 3.24)

project(G-PIES)

set(BUILD_CUDA false CACHE BOOL "Build CUDA-dependent targets.")
set(CUDA_ARCHITECTURES native CACHE STRING "CUDA architectures.")
add_compile_options("$<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda>")
if(BUILD_CUDA)
  enable_language(CUDA)
endif()

set(BUILD_METAL false CACHE BOOL "Build Metal-dependent targets.")
if(BUILD_METAL)
  include(FetchContent)
  FetchContent_Declare(
    metal-cpp
    URL https://developer.apple.com/metal/cpp/files/metal-cpp_macOS14.2_iOS17.2.zip
  )
  FetchContent_MakeAvailable(metal-cpp)
  set(METAL_INCLUDE_DIRS ${metal-cpp_SOURCE_DIR})
endif()

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(./cli)
add_subdirectory(./src/client_db)
add_subdirectory(./src/cluster_dynamics/cpu)
add_subdirectory(./src/cluster_dynamics/cuda)
add_subdirectory(./src/cluster_dynamics/metal)
add_subdirectory(./test)
