file(GLOB SRC_FILES ./*.cpp)
add_executable(test_clientdb ${SRC_FILES})
target_link_libraries(test_clientdb clientdb)
target_link_libraries(test_clientdb clusterdynamics)
target_link_libraries(test_clientdb GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_clientdb)
gpies_add_code_coverage_target(test_clientdb)
