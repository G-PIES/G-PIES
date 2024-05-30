set(GP_BUILD_CUDA false CACHE BOOL "Build CUDA-dependent targets.")
set(CUDA_ARCHITECTURES native CACHE STRING "CUDA architectures.")
if(GP_BUILD_CUDA)
  # This variable is required by SUNDIALS
  set(ENABLE_CUDA ON)
  set(CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCHITECTURES})
  enable_language(CUDA)
endif()
