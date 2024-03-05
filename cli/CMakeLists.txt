add_executable(cd_cli ./cd_cli.cpp)
target_include_directories(cd_cli PRIVATE ../src/cluster_dynamics)
target_link_libraries(cd_cli clusterdynamics)

add_executable(db_cli ./db_cli.cpp)
target_include_directories(db_cli PRIVATE ${SQLite3_INCLUDE_DIRS})
target_include_directories(db_cli PRIVATE ../src/cluster_dynamics)
target_link_libraries(db_cli clientdb)
target_link_libraries(db_cli clusterdynamics)

if(GP_BUILD_CUDA)
  add_executable(cd_cuda_cli ./cd_cli.cpp)
  target_compile_definitions(cd_cuda_cli PRIVATE USE_CUDA)
  target_include_directories(cd_cuda_cli PRIVATE ../src/cluster_dynamics)
  target_link_libraries(cd_cuda_cli clusterdynamicscuda)
endif()

if(GP_BUILD_METAL)
  add_executable(cd_metal_cli ./cd_cli.cpp)
  target_compile_definitions(cd_metal_cli PRIVATE USE_METAL)
  target_include_directories(cd_metal_cli PRIVATE ../src/cluster_dynamics)
  target_link_libraries(cd_metal_cli clusterdynamicsmetal)
  target_link_libraries(cd_metal_cli
    "-framework Metal"
    "-framework Foundation"
    "-framework QuartzCore"
  )
endif()