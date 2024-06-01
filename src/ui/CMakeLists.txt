find_package(pybind11 CONFIG)

if(NOT pybind11_FOUND)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.12.0
)

FetchContent_MakeAvailable(pybind11)
endif()

add_library(pyclusterdynamics SHARED ./wrapper.cpp)
target_include_directories(pyclusterdynamics PRIVATE $(pybind11_INCLUDE_DIR))
target_link_libraries(pyclusterdynamics yaml-cpp::yaml-cpp)
target_link_libraries(pyclusterdynamics clusterdynamics)
target_link_libraries(pyclusterdynamics pybind11::module)
target_compile_options(pyclusterdynamics PRIVATE "-fPIC")