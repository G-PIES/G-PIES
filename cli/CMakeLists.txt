add_executable(cd_cli ./cd_cli.cpp)
target_include_directories(cd_cli PRIVATE ../src/cluster_dynamics)
target_link_libraries(cd_cli clusterdynamics)
target_link_libraries(cd_cli clientdb)
target_link_libraries(cd_cli Boost::program_options)
target_link_libraries(cd_cli yaml-cpp::yaml-cpp) 

add_executable(db_cli ./db_cli.cpp)
target_include_directories(db_cli PRIVATE ../src/cluster_dynamics)
target_link_libraries(db_cli clientdb)
target_link_libraries(db_cli clusterdynamics)

if(GP_BUILD_CUDA)
  add_executable(cd_cuda_cli ./cd_cli.cpp)
  target_compile_definitions(cd_cuda_cli PRIVATE USE_CUDA)
  target_include_directories(cd_cuda_cli PRIVATE ../src/cluster_dynamics)
  target_link_libraries(cd_cuda_cli clusterdynamicscuda)
  target_link_libraries(cd_cuda_cli clientdb)
  target_link_libraries(cd_cuda_cli Boost::program_options)
  target_link_libraries(cd_cuda_cli yaml-cpp::yaml-cpp) 
endif()

if(GP_BUILD_METAL)
  add_executable(cd_metal_cli ./cd_cli.cpp)
  target_compile_definitions(cd_metal_cli PRIVATE USE_METAL)
  target_compile_definitions(cd_metal_cli PUBLIC GP_FLOAT=float)
  target_include_directories(cd_metal_cli PRIVATE ../src/cluster_dynamics)
  target_link_libraries(cd_metal_cli clusterdynamicsmetal)
  target_link_libraries(cd_metal_cli clientdbmetal)
  target_link_libraries(cd_metal_cli
    "-framework Metal"
    "-framework Foundation"
    "-framework QuartzCore"
  )
  target_link_libraries(cd_metal_cli Boost::program_options)
endif()
