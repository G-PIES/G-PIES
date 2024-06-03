if(GP_TEST_COVERAGE)
  setup_target_for_coverage_gcovr_html(
    NAME test_coverage
    BASE_DIRECTORY ${PROJECT_SOURCE_DIR}
    DEPENDENCIES ${GP_CODE_COVERAGE_TARGETS}
    EXCLUDE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/src/okmc ${PROJECT_SOURCE_DIR}/test
  )
endif()
