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
