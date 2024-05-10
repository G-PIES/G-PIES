file(GLOB SRC_FILES ./*.cpp ../*.cpp)

add_library(clusterdynamics STATIC ${SRC_FILES})
target_link_libraries(clusterdynamics PUBLIC SUNDIALS::cvode SUNDIALS::nvecserial)
target_include_directories(clusterdynamics PRIVATE ./)
target_include_directories(clusterdynamics PRIVATE ../)
set_target_properties(clusterdynamics PROPERTIES POSITION_INDEPENDENT_CODE ON)