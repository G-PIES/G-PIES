cmake_minimum_required(VERSION 3.20)
include(FetchContent)

project(G-PIES)

# Use the new behavior of DOWNLOAD_EXTRACT_TIMESTAMP if CMake supports it
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

set(GP_BUILD_CUDA false CACHE BOOL "Build CUDA-dependent targets.")
set(CUDA_ARCHITECTURES native CACHE STRING "CUDA architectures.")
add_compile_options("$<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda>")
if(GP_BUILD_CUDA)
  # This variable is required by SUNDIALS
  set(ENABLE_CUDA ON)
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

find_package(SUNDIALS CONFIG)
if(NOT SUNDIALS_FOUND)
  set(EXAMPLES_ENABLE_C OFF)
  set(BUILD_ARKODE OFF)
  set(BUILD_IDA OFF)
  set(BUILD_IDAS OFF)
  set(BUILD_KINSOL OFF)
  set(BUILD_SHARED_LIBS OFF)
  FetchContent_Declare(
    SUNDIALS
    URL https://github.com/LLNL/sundials/releases/download/v7.0.0/sundials-7.0.0.tar.gz
  )
  
  set(HOLDER ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/.build/lib)
  FetchContent_MakeAvailable(SUNDIALS)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${HOLDER})
endif()

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
set(CMAKE_CUDA_STANDARD 20)

add_compile_definitions($<$<PLATFORM_ID:Windows>:_USE_MATH_DEFINES>)

# TODO: Consider replacing /W2 with /W4 or /Wall for MSVC.
add_compile_options("$<$<AND:$<NOT:$<COMPILE_LANGUAGE:CUDA>>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-Wall;-Wextra;-Werror>")
add_compile_options("$<$<AND:$<NOT:$<COMPILE_LANGUAGE:CUDA>>,$<CXX_COMPILER_ID:MSVC>>:/W2;/WX>")
add_compile_options("$<$<COMPILE_LANGUAGE:CUDA>:-Werror;all-warnings>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CUDA>>:SHELL:-Xcompiler /W2>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CUDA>>:SHELL:-Xcompiler /WX>")

set(GP_NO_SANITIZER false CACHE BOOL "Disable sanitizers for debug builds.")
if(NOT GP_NO_SANITIZER)
  # TODO: Configure sanitizer for MSVC. MSVC requires manually copying ASAN DLLs to the output folder.
  # add_compile_options("$<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fsanitize=undefined;-fsanitize=address>")
  # add_link_options("$<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fsanitize=address;-fsanitize=undefined>")
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_subdirectory(./cli)
add_subdirectory(./src/client_db)
add_subdirectory(./src/cluster_dynamics/cpu)
add_subdirectory(./src/cluster_dynamics/cuda)
add_subdirectory(./src/cluster_dynamics/metal)
add_subdirectory(./src/okmc)
add_subdirectory(./test)
