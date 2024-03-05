file(GLOB SRC_FILES ./*.cpp ../*.cpp)

add_library(clusterdynamics STATIC ${SRC_FILES})
target_include_directories(clusterdynamics PRIVATE ./)
target_include_directories(clusterdynamics PRIVATE ../)
