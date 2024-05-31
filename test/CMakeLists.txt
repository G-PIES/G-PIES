include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e15c5a51b818baedc8e33197d2427bc9568e110c.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(gpies_tests ./gpies_tests.cpp)
target_include_directories(gpies_tests PRIVATE ./)
target_include_directories(gpies_tests PRIVATE ../src/cluster_dynamics)
target_include_directories(gpies_tests PRIVATE ../src/cluster_dynamics/cpu)
target_link_libraries(gpies_tests clusterdynamics)
target_link_libraries(gpies_tests GTest::gtest_main)

if(GP_BUILD_CUDA)
  add_executable(gpies_cuda_tests ./gpies_tests.cpp)
  set_source_files_properties(./gpies_tests.cpp PROPERTIES LANGUAGE CUDA)
  target_compile_definitions(gpies_cuda_tests PRIVATE USE_CUDA)
  target_include_directories(gpies_cuda_tests PRIVATE ./)
  target_include_directories(gpies_cuda_tests PRIVATE ../src/cluster_dynamics)
  target_include_directories(gpies_cuda_tests PRIVATE ../src/cluster_dynamics/cuda)
  target_link_libraries(gpies_cuda_tests clusterdynamics)
  target_link_libraries(gpies_cuda_tests GTest::gtest_main)
endif()

add_executable(db_tests ./db_tests.cpp)
target_include_directories(db_tests PRIVATE ./)
target_include_directories(db_tests PRIVATE ../src/cluster_dynamics)
target_include_directories(db_tests PRIVATE ../src/cluster_dynamics/cpu)
target_include_directories(db_tests PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(db_tests clientdb)
target_link_libraries(db_tests clusterdynamics)
target_link_libraries(db_tests GTest::gtest_main)
