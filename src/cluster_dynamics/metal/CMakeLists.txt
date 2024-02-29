if(BUILD_METAL)

  file(GLOB SRC_FILES ./*.cpp ../*.cpp)
  add_library(clusterdynamicsmetal STATIC ${SRC_FILES})
  target_compile_definitions(clusterdynamicsmetal PUBLIC USE_METAL)
  target_compile_definitions(clusterdynamicsmetal PRIVATE METALLIB_PATH="${PROJECT_ROOT}/out/cluster_dynamics.metallib")
  target_include_directories(clusterdynamicsmetal PRIVATE ${PROJECT_ROOT}/include/utils)
  target_include_directories(clusterdynamicsmetal PRIVATE ./)
  target_include_directories(clusterdynamicsmetal PRIVATE ../)
  target_include_directories(clusterdynamicsmetal PRIVATE ${METAL_INCLUDE_DIRS})
  add_dependencies(clusterdynamicsmetal clusterdynamicsmetalshaders)

  add_custom_target(
    clusterdynamicsmetalshaders
    COMMAND
      xcrun -sdk macosx metal
        -DGP_FLOAT=float
        -I${PROJECT_ROOT}/include
        -I${PROJECT_ROOT}/src/cluster_dynamics
        -I${PROJECT_ROOT}/src/cluster_dynamics/metal
        ${PROJECT_ROOT}/shader/metal/cluster_dynamics.metal
        -o ${PROJECT_ROOT}/out/cluster_dynamics.metallib
    BYPRODUCTS ${PROJECT_ROOT}/out/cluster_dynamics.metallib
    SOURCES ${PROJECT_ROOT}/shader/metal/cluster_dynamics.metal)

endif()
