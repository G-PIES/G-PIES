file(GLOB_RECURSE SRC_FILES ./*.cpp)

add_executable(okmc ${SRC_FILES})
target_include_directories(okmc PRIVATE ./)
