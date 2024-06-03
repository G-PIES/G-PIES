set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_STANDARD 20)

if(GP_BUILD_CUDA)
  add_compile_definitions(USE_CUDA)
  include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
endif()

add_compile_definitions($<$<PLATFORM_ID:Windows>:_USE_MATH_DEFINES>)

add_compile_options("$<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda>")

# TODO: Consider replacing /W2 with /W4 or /Wall for MSVC.
add_compile_options("$<$<AND:$<NOT:$<COMPILE_LANGUAGE:CUDA>>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-Wall;-Wextra;-Werror>")
add_compile_options("$<$<AND:$<NOT:$<COMPILE_LANGUAGE:CUDA>>,$<CXX_COMPILER_ID:MSVC>>:/W2;/WX>")
add_compile_options("$<$<COMPILE_LANGUAGE:CUDA>:-Werror;all-warnings>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CUDA>>:SHELL:-Xcompiler /W2>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CUDA>>:SHELL:-Xcompiler /WX>")

set(GP_NO_SANITIZER false CACHE BOOL "Disable sanitizers for debug builds.")
if(NOT GP_NO_SANITIZER AND NOT GP_BUILD_CUDA)
  # TODO: Configure sanitizer for MSVC. MSVC requires manually copying ASAN DLLs to the output folder.
  add_compile_options("$<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<NOT:$<COMPILE_LANGUAGE:CUDA>>>:-fsanitize=undefined;-fsanitize=address>")
  add_link_options("$<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<NOT:$<COMPILE_LANGUAGE:CUDA>>>:-fsanitize=address;-fsanitize=undefined>")
endif()
if(NOT GP_NO_SANITIZER AND GP_BUILD_CUDA)
  # TODO: Fix sanitizers for non-cuda code
  message(WARNING "Sanitizers are disabled for CUDA builds.")
endif()

if(GP_TEST_COVERAGE)
  set(GCOVR_ADDITIONAL_ARGS "--txt")
  append_coverage_compiler_flags()
endif()

set(GP_CODE_COVERAGE_TARGETS "")
function(gpies_add_code_coverage_target name)
  list(APPEND GP_CODE_COVERAGE_TARGETS ${name})
endfunction()