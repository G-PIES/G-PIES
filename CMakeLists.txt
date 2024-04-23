cmake_minimum_required(VERSION 3.20)
include(FetchContent)

project(G-PIES)

set(GP_BUILD_CUDA false CACHE BOOL "Build CUDA-dependent targets.")
set(CUDA_ARCHITECTURES native CACHE STRING "CUDA architectures.")
add_compile_options("$<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda>")
if(GP_BUILD_CUDA)
  enable_language(CUDA)
endif()

set(GP_BUILD_METAL false CACHE BOOL "Build Metal-dependent targets.")
if(GP_BUILD_METAL)
  FetchContent_Declare(
    metal-cpp
    URL https://developer.apple.com/metal/cpp/files/metal-cpp_macOS14.2_iOS17.2.zip
  )
  FetchContent_MakeAvailable(metal-cpp)
  set(METAL_INCLUDE_DIRS ${metal-cpp_SOURCE_DIR})
endif()

find_package(SUNDIALS REQUIRED)

set(Boost_USE_STATIC_LIBS       ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_USE_MULTITHREADED     OFF)
find_package(Boost COMPONENTS program_options)
if(NOT Boost_FOUND)
  FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z # downloading a zip release speeds up the download
  )
  set(BOOST_ENABLE_CMAKE ON)
  set(BOOST_INCLUDE_LIBRARIES program_options)
  FetchContent_MakeAvailable(Boost)
endif()

find_package(SQLite3)
if(SQLite3_FOUND)
  set(SQLite3_LIBRARY_TARGET SQLite::SQLite3)
else()
  FetchContent_Declare(
    sqlite3
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
  )
  FetchContent_MakeAvailable(sqlite3)
  add_library(sqlite3_lib ${sqlite3_SOURCE_DIR}/sqlite3.c)
  set(SQLite3_INCLUDE_DIRS ${sqlite3_SOURCE_DIR})
  set(SQLite3_LIBRARY_TARGET sqlite3_lib)
endif()

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)

set(CMAKE_CXX_STANDARD 20)
add_compile_definitions($<$<PLATFORM_ID:Windows>:_USE_MATH_DEFINES>)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_subdirectory(./cli)
add_subdirectory(./src/client_db)
add_subdirectory(./src/cluster_dynamics/cpu)
add_subdirectory(./src/cluster_dynamics/cuda)
add_subdirectory(./src/cluster_dynamics/metal)
add_subdirectory(./src/okmc)
add_subdirectory(./test)
