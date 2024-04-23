if(GP_BUILD_CUDA)

  file(GLOB SRC_FILES ./*.cpp ../*.cpp)

  add_library(clusterdynamicscuda STATIC ${SRC_FILES})
  set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CUDA)
  target_link_libraries(
    clusterdynamicscuda PUBLIC 
    cusparse cusolver 
    SUNDIALS::cvode SUNDIALS::nveccuda_static 
    SUNDIALS::sunlinsolcusolversp_static SUNDIALS::sunmatrixcusparse_static
  )
  target_compile_definitions(clusterdynamicscuda PRIVATE USE_CUDA)
  target_include_directories(clusterdynamicscuda PRIVATE ./)
  target_include_directories(clusterdynamicscuda PRIVATE ../)

endif()
