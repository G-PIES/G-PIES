cmake_minimum_required(VERSION 3.5.0)

project(DiffEqSolverTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(SUNDIALS REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(generator ./src_generator/main.cpp ./src_generator/generator.cpp ./src_generator/expression.cpp ./src_generator/parse.cpp ./src_generator/tokenize.cpp)

add_executable(solver ./src_solver/main.cpp)
target_link_libraries(solver SUNDIALS::cvode SUNDIALS::nvecserial)

add_executable(tests ./test/test.cpp ./src_generator/tokenize.cpp ./src_generator/parse.cpp ./src_generator/expression.cpp)
target_link_libraries(tests GTest::gtest_main)