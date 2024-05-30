FetchContent_Declare(
  cmake-modules
  GIT_REPOSITORY https://github.com/bilke/cmake-modules.git
  GIT_TAG master 
)

FetchContent_MakeAvailable(cmake-modules)

set(GP_TEST_COVERAGE false CACHE BOOL "Calculate test coverage stats.")
if(GP_TEST_COVERAGE)
  get_property(LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
  message("=== LANGUAGES = ${LANGUAGES}")
  foreach(LANG ${LANGUAGES})
    message("LANG = ${LANG} COMPILER_ID = ${CMAKE_${LANG}_COMPILER_ID}")
  endforeach()
  include(${cmake-modules_SOURCE_DIR}/CodeCoverage.cmake)
endif()
