file(GLOB SRC_FILES ./*.cpp)

add_library(clientdb STATIC ${SRC_FILES})
target_include_directories(clientdb PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(clientdb ${SQLite3_LIBRARY_TARGET})

if(GP_BUILD_METAL)

  add_library(clientdbmetal STATIC ${SRC_FILES})
  target_compile_definitions(clientdbmetal PUBLIC USE_METAL)
  target_compile_definitions(clientdbmetal PUBLIC GP_FLOAT=float)
  target_include_directories(clientdbmetal PRIVATE ${SQLite3_INCLUDE_DIRS})
  target_link_libraries(clientdbmetal ${SQLite3_LIBRARY_TARGET})

endif()
