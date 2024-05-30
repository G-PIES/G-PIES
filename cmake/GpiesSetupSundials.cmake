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
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
  FetchContent_MakeAvailable(SUNDIALS)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${HOLDER})
endif()
