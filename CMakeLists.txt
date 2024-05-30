cmake_minimum_required(VERSION 3.20)

project(G-PIES)

# Use the new behavior of DOWNLOAD_EXTRACT_TIMESTAMP if CMake supports it
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

find_program(HOMEBREW brew)
if(HOMEBREW)
  execute_process(COMMAND brew --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX)
  string(REPLACE "\n" "" HOMEBREW_PREFIX ${HOMEBREW_PREFIX})
  link_directories(${HOMEBREW_PREFIX}/lib)
endif()

include(cmake/GpiesSetupCuda.cmake)
include(cmake/GpiesSetupDependencies.cmake)
include(cmake/GpiesSetupCompilers.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(./cli)
add_subdirectory(./src/client_db)
add_subdirectory(./src/cluster_dynamics)
add_subdirectory(./src/okmc)
add_subdirectory(./test)
