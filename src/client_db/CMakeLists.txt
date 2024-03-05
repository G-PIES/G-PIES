file(GLOB SRC_FILES ./*.cpp)
add_library(clientdb STATIC ${SRC_FILES})
target_include_directories(clientdb PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(clientdb ${SQLite3_LIBRARY_TARGET})
