include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e15c5a51b818baedc8e33197d2427bc9568e110c.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(db_tests ./db_tests.cpp)
target_include_directories(db_tests PRIVATE ./)
target_include_directories(db_tests PRIVATE ../src/cluster_dynamics)
target_include_directories(db_tests PRIVATE ../src/cluster_dynamics/cpu)
target_include_directories(db_tests PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(db_tests clientdb)
target_link_libraries(db_tests clusterdynamics)
target_link_libraries(db_tests GTest::gtest_main)
