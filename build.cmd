@echo off

setlocal EnableDelayedExpansion

set this_script=%0

where cmake > nul 2>&1
if errorlevel 1 (
  echo This script requires cmake to be installed and added to PATH.
  endlocal
  exit /b 1
)

:options_loop
if "%1" neq "" (
  set option=%1
  if "!option:~0,2!" equ "--" (
    call :process_option !option!
  ) else if "!option:~0,1!" equ "-" (
    call :process_single_dash_options !option!
  ) else (
    call :process_target !option!
  )
  shift
  goto :options_loop
)

if "%option_help%" neq "" (
  echo Usage:
  echo   %this_script% [options] [targets]
  echo.
  echo Examples:
  echo.
  echo   # Build all CPU targets"
  echo   %this_script%
  echo.
  echo   # Build and run Cluster Dynamics CLI with CUDA
  echo   %this_script% --cuda --run gpies
  echo.
  echo Targets:
  echo   cd                  The Cluster Dynamics library
  echo   gpies               The CLI for the Cluster Dynamics library
  echo   db                  The DB Library
  echo   dbcli               The CLI for the DB library
  echo   dbtests             GoogleTest based tests for DB library
  echo   okmc                The OKMC application
  echo.
  echo Options:
  echo   --help              Show this help.
  echo   --run, -r           Run target after build. Must be only one runnable target.
  echo   --clean, -c         Clean everything before building the project.
  echo                       Removes build, out, and db directories.
  echo   --force, -f         Do not ask for confirmation when --clean is specified.
  echo   --cpu               Build CPU targets.
  echo                       This option is assumed if no --cuda specified.
  echo   --cuda              Build CUDA targets for the current GPU architecture.
  echo                       ^(nvcc -arch=native^)
  echo   --cuda-all-major    Build CUDA targets for all major GPU architectures.
  echo                       ^(nvcc -arch=all-major^)
  echo   --debug             Build debug build ^(optimizations and sanitizer turned on^).
  echo                       Cannot be usage together with --release.
  echo   --release           Build release build ^(max optimizations^).
  echo                       Cannot be usage together with --debug.
  echo   --no-sanitizer      Do not use sanitizer for debug builds.
  echo   --cmake-verbose     Enable verbose output in the build process.
  echo                       ^(CMAKE_VERBOSE_MAKEFILE=ON^)
  echo   --test-coverage     Calculate unit tests coverage.
  goto :exit
)

if "%option_debug%%option_release%" equ "11" (
  call :echo_error "Both --debug and --release cannot be used at the same time."
)

if "%test_coverage%%option_release%" equ "11" (
  call :echo_error "Both --test-coverage and --release cannot be used at the same time."
)

if "%option_cuda%" equ "" (
  set option_cpu=1
)

if "%option_verbose%%option_csv%" equ "11" (
  call :echo_error "Both --verbose and --csv cannot be used at the same time."
)

if "%option_cpu%" neq "" (
  set targets_to_build=%targets_to_build% %cpu_targets%
  set targets_to_run=%targets_to_run% %cpu_runnable_targets%
)

if "%option_cuda%" neq "" (
  set targets_to_build=%targets_to_build% %cuda_targets%
  set targets_to_run=%targets_to_run% %cuda_runnable_targets%
)

set targets_to_build=%targets_to_build% %targets_to_run%

if "%unknown_targets%" neq "" (
  call :echo_error "Unknown targets:%unknown_targets%"
) else (
  set has_target_to_build=0
  for %%t in (%targets%) do set has_targets=1
  for %%t in (%targets_to_build%) do set has_target_to_build=1
  if "!has_targets!!has_target_to_build!" equ "10" (
    call :echo_error "No targets to build. You might forgot to specify --cpu."
  )
  if "!has_targets!!test_coverage!" equ "11" (
    call :echo_error  "--test-coverage cannot be used for specific targets." 
  )
)

set count_targets_to_run=0
for %%t in (%targets_to_run%) do (
  set /a count_targets_to_run=count_targets_to_run+1
  set target_to_run=%%t
)
if "%count_targets_to_run%" gtr 1 (
  call :echo_error "More than one target to run. Runnable targets:%targets_to_run%"
)

if "%error%" neq "" (
  goto :exit
)

set cmake_configure_options=-B .build -S .
set cmake_build_options=--build .build -j 4

if "%option_release%" neq "" (
  set cmake_configure_options=%cmake_configure_options% -DCMAKE_BUILD_TYPE=Release
  set cmake_build_options=%cmake_build_options% --config Release
  set out_path=out\Release
) else (
  set cmake_configure_options=%cmake_configure_options% -DCMAKE_BUILD_TYPE=Debug
  set cmake_build_options=%cmake_build_options% --config Debug
  set out_path=out\Debug
)

if "%option_cuda%" neq "" (
  set cmake_configure_options=%cmake_configure_options% -DGP_BUILD_CUDA=true
) else (
  set cmake_configure_options=%cmake_configure_options% -DGP_BUILD_CUDA=false
)

if "%option_cuda_all%" neq "" (
  set cmake_configure_options=%cmake_configure_options% -DCUDA_ARCHITECTURES=all-major
) else (
  set cmake_configure_options=%cmake_configure_options% -DCUDA_ARCHITECTURES=native
)

if "%no_sanizier%" neq "" (
  set cmake_configure_options=%cmake_configure_options% -DGP_NO_SANITIZER:BOOL=true
) else (
  set cmake_configure_options=%cmake_configure_options% -DGP_NO_SANITIZER:BOOL=false
)

if "%cmake_verbose%" neq "" (
  set cmake_configure_options=%cmake_configure_options% -DCMAKE_VERBOSE_MAKEFILE:BOOL=true
) else (
  set cmake_configure_options=%cmake_configure_options% -DCMAKE_VERBOSE_MAKEFILE:BOOL=false
)

if "%test_coverage%" neq "" (
  set cmake_configure_options=%cmake_configure_options% -DGP_TEST_COVERAGE:BOOL=true
) else (
  set cmake_configure_options=%cmake_configure_options% -DGP_TEST_COVERAGE:BOOL=false
)

for %%t in (%targets_to_build%) do (
  set cmake_build_options=!cmake_build_options! --target %%t
)

if "%option_clean%" neq "" (
  if "%option_force%" equ "" (
    echo Are you sure you want to clean all build artifacts?
    echo This will delete the following folders: .build, out, db.
    set /p "option_force=Enter Y to confirm: "
    if /i "!option_force!" equ "y" (
      set option_force=1
    )
  )
  if "!option_force!" equ "1" (
    if exist .build rmdir /s /q .build
    if exist out rmdir /s /q out
    if exist db rmdir /s /q db
  ) else (
    goto :exit
  )
)

call cmake %cmake_configure_options%
if errorlevel 1 (
  call :echo_error "cmake returned %ERRORLEVEL%"
  goto :exit
)

call cmake %cmake_build_options%
if errorlevel 1 (
  call :echo_error "cmake returned %ERRORLEVEL%"
  goto :exit
)

if "%test_coverage%" neq "" (
  call cmake %cmake_build_options% --target test test_coverage
  if errorlevel 1 (
    call :echo_error "cmake returned %ERRORLEVEL%"
    goto :exit
  )
)

if "%option_run%" neq "" (
  call %out_path%\%target_to_run%.exe %run_options%
  if errorlevel 1 (
    set error=1
  )
)

:exit
  if "%error%" neq "" (
    endlocal
    exit /b 1
  ) else (
    endlocal
    exit /b 0
  )

:process_single_dash_options
  set option=%1
  set i=1
  :process_single_dash_options_loop
  if "!option:~%i%,1!" neq "" (
    call :process_option -!option:~%i%,1!
    set /a i=i+1
    goto :process_single_dash_options_loop
  )
goto :eof

:process_option
  if "%1" equ "--help"           set "option_help=1"    && goto :eof
  if "%1" equ "-h"               set "option_help=1"    && goto :eof
  if "%1" equ "--run"            set "option_run=1"     && goto :eof
  if "%1" equ "-r"               set "option_run=1"     && goto :eof
  if "%1" equ "--clean"          set "option_clean=1"   && goto :eof
  if "%1" equ "-c"               set "option_clean=1"   && goto :eof
  if "%1" equ "--force"          set "option_force=1"   && goto :eof
  if "%1" equ "-f"               set "option_force=1"   && goto :eof
  if "%1" equ "--verbose"        set "option_verbose=1" && goto :eof
  if "%1" equ "-v"               set "option_verbose=1" && goto :eof
  if "%1" equ "--csv"            set "option_csv=1"     && goto :eof
  if "%1" equ "-c"               set "option_clean=1"   && goto :eof
  if "%1" equ "--clean"          set "option_clean=1"   && goto :eof
  if "%1" equ "--cpu"            set "option_cpu=1"     && goto :eof
  if "%1" equ "--cuda"           set "option_cuda=1"    && goto :eof
  if "%1" equ "--cuda-all-major" set "option_cuda=1" && set "option_cuda_all=1" && goto :eof
  if "%1" equ "--debug"          set "option_debug=1"   && goto :eof
  if "%1" equ "--release"        set "option_release=1" && goto :eof
  if "%1" equ "--no-sanitizer"   set "no_sanitizer=1"   && goto :eof
  if "%1" equ "--cmake-verbose"  set "cmake_verbose=1"  && goto :eof
  if "%1" equ "--test-coverage"  set "test_coverage=1"  && goto :eof
  call :echo_error "Unknown option %1"
goto :eof

:process_target
  set targets=%targets% %1
  if "%1" equ "cd" (
    set cpu_targets=%cpu_targets% clusterdynamics
    set cuda_targets=%cuda_targets% clusterdynamicscuda
  ) else if "%1" equ "gpies" (
    set cpu_runnable_targets=%cpu_runnable_targets% gpies
    set cuda_runnable_targets=%cuda_runnable_targets% gpies
  ) else if "%1" equ "db" (
    set cpu_targets=%cpu_targets% clientdb
  ) else if "%1" equ "dbcli" (
    set cpu_runnable_targets=%cpu_runnable_targets% db_cli
  ) else if "%1" equ "dbtests" (
    set cpu_runnable_targets=%cpu_runnable_targets% test_clientdb
  ) else if "%1" equ "okmc" (
    set cpu_runnable_targets=%cpu_runnable_targets% okmc
  ) else (
    set unknown_targets=%unknown_targets% %1
  )
goto :eof

:help
goto :eof

:echo_error
  cmake -E cmake_echo_color --red %1 >&2
  set error=1
goto :eof
