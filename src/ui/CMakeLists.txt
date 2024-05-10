find_package(pybind11 REQUIRED CONFIG)

add_library(pyclusterdynamics SHARED ./wrapper.cpp)
target_include_directories(pyclusterdynamics PRIVATE $(pybind11_INCLUDE_DIR))
target_link_libraries(pyclusterdynamics clusterdynamics)
target_link_libraries(pyclusterdynamics pybind11::module)