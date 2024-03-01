if(BUILD_CUDA)

  file(GLOB SRC_FILES ./*.cpp ../*.cpp)

  add_library(clusterdynamicscuda STATIC ${SRC_FILES})
  set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CUDA)
  target_compile_definitions(clusterdynamicscuda PRIVATE USE_CUDA)
  target_include_directories(clusterdynamicscuda PRIVATE ./)
  target_include_directories(clusterdynamicscuda PRIVATE ../)

endif()
