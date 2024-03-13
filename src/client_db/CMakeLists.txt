file(GLOB SRC_FILES ./*.cpp)
add_library(clientdb STATIC ${SRC_FILES})
target_include_directories(clientdb PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(clientdb ${SQLite3_LIBRARY_TARGET})

if(GP_BUILD_METAL)
  target_compile_definitions(clientdb PUBLIC USE_METAL)
endif()