cmake_minimum_required(VERSION 3.20)
include(FetchContent)

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

set(GP_BUILD_CUDA false CACHE BOOL "Build CUDA-dependent targets.")
set(CUDA_ARCHITECTURES native CACHE STRING "CUDA architectures.")
set(CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCHITECTURES})
add_compile_options("$<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda>")
if(GP_BUILD_CUDA)
  # This variable is required by SUNDIALS
  set(ENABLE_CUDA ON)
  enable_language(CUDA)
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

find_package(BZip2)
if (NOT BZIP2_FOUND)
  FetchContent_Declare(
    BZip2
    GIT_REPOSITORY https://gitlab.com/bzip2/bzip2
    GIT_TAG 66c46b8c9436613fd81bc5d03f63a61933a4dcc3)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  set(ENABLE_LIB_ONLY   ON)
  set(ENABLE_STATIC_LIB ON)
  set(ENABLE_TESTS      OFF)
  FetchContent_MakeAvailable(BZip2)
  set(BZIP2_LIBRARIES bz2_static)
  set(BZIP2_INCLUDE_DIR ${bzip2_SOURCE_DIR})
  include_directories(${BZIP2_INCLUDE_DIR})
  add_library(BZip2::BZip2 ALIAS ${BZIP2_LIBRARIES})
endif()

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(Boost_USE_MULTITHREADED      OFF)
set(BOOST_IOSTREAMS_ENABLE_BZIP2 ON)
find_package(Boost COMPONENTS program_options iostreams)
if(NOT Boost_FOUND)
  FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z # downloading a zip release speeds up the download
  )
  set(BOOST_ENABLE_CMAKE ON)
  set(BOOST_INCLUDE_LIBRARIES program_options iostreams)
  FetchContent_MakeAvailable(Boost)
endif()

find_package(SQLite3)
if(NOT SQLite3_FOUND)
  FetchContent_Declare(
    sqlite3
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
  )
  FetchContent_MakeAvailable(sqlite3)
  add_library(sqlite3_lib ${sqlite3_SOURCE_DIR}/sqlite3.c)
  set(SQLite3_INCLUDE_DIRS ${sqlite3_SOURCE_DIR})
  set(SQLite3_LIBRARIES sqlite3_lib)
  add_library(SQLite::SQLite3 ALIAS ${SQLite3_LIBRARIES})
endif()

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_GetProperties(yaml-cpp)

if(NOT yaml-cpp_POPULATED)
  message(STATUS "Fetching yaml-cpp...")
  FetchContent_Populate(yaml-cpp)
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_STANDARD 20)

if(GP_BUILD_CUDA)
  add_compile_definitions(USE_CUDA)
  include_directories(clusterdynamics PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
endif()

add_compile_definitions($<$<PLATFORM_ID:Windows>:_USE_MATH_DEFINES>)

# TODO: Consider replacing /W2 with /W4 or /Wall for MSVC.
add_compile_options("$<$<AND:$<NOT:$<COMPILE_LANGUAGE:CUDA>>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-Wall;-Wextra;-Werror;-fPIC>")
add_compile_options("$<$<AND:$<NOT:$<COMPILE_LANGUAGE:CUDA>>,$<CXX_COMPILER_ID:MSVC>>:/W2;/WX>")
add_compile_options("$<$<COMPILE_LANGUAGE:CUDA>:-Werror;all-warnings>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CUDA>>:SHELL:-Xcompiler /W2>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CUDA>>:SHELL:-Xcompiler /WX>")

set(GP_NO_SANITIZER false CACHE BOOL "Disable sanitizers for debug builds.")
if(NOT GP_NO_SANITIZER AND NOT GP_BUILD_CUDA)
  # TODO: Configure sanitizer for MSVC. MSVC requires manually copying ASAN DLLs to the output folder.
  add_compile_options("$<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<NOT:$<COMPILE_LANGUAGE:CUDA>>>:-fsanitize=undefined;-fsanitize=address>")
  add_link_options("$<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<NOT:$<COMPILE_LANGUAGE:CUDA>>>:-fsanitize=address;-fsanitize=undefined>")
endif()
if(NOT GP_NO_SANITIZER AND GP_BUILD_CUDA)
  # TODO: Fix sanitizers for non-cuda code
  message(WARNING "Sanitizers are disabled for CUDA builds.")
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_subdirectory(./cli)
add_subdirectory(./src/client_db)
add_subdirectory(./src/cluster_dynamics)
add_subdirectory(./src/okmc)
add_subdirectory(./src/ui)
add_subdirectory(./test)
