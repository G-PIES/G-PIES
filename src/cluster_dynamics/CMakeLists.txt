file(GLOB SRC_FILES ./*.cpp ./cpu/*.cpp)

add_library(clusterdynamics STATIC ${SRC_FILES})
target_link_libraries(clusterdynamics PUBLIC SUNDIALS::cvode SUNDIALS::nvecserial)
target_include_directories(clusterdynamics PRIVATE .)
gpies_add_code_coverage_target(clusterdynamics)

if(GP_BUILD_CUDA)

  file(GLOB SRC_FILES ./cuda/*.cpp)
  target_sources(clusterdynamics PRIVATE ${SRC_FILES})
  set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CUDA)

endif()
