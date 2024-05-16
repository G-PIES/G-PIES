#!/bin/bash

set -e

if ! command -v cmake &> /dev/null; then
  >&2 echo "This script requires cmake to be installed and added to PATH."
  exit 1
fi

echo_error() {
  >&2 cmake -E cmake_echo_color --red "$1"
  ERROR=1
}

process_option() {
  option=$1
  case $option in
    --run|-r) RUN=1 ;;
    --clean|-c) CLEAN=1 ;;
    --force|-f) FORCE=1 ;;
    --cpu) CPU=1 ;;
    --cuda) CUDA=1 ;;
    --cuda-all-major) CUDA=1; CUDA_ALL=1 ;;
    --debug) DEBUG=1 ;;
    --release) RELEASE=1 ;;
    --help|-h) HELP=1 ;;
    --no-sanitizer) NO_SANITIZER=1 ;;
    --cmake-verbose) CMAKE_VERBOSE=1 ;;
    *)
      echo_error "Unknown option $1"
      HELP=1
      ;;
  esac
}

while [[ $# -gt 0 ]]; do
  option=$1

  case $option in
    --*)
      process_option $1
      shift
      ;;
    -*)
      for (( i=1; i<${#1}; i++ )); do
        process_option -${1:$i:1}
      done
      shift
      ;;
    *)
      TARGETS+=("$1")
      shift
      ;;
  esac
done

if [ "$HELP" ]; then
  echo "Usage:"
  echo "  $0 [options] [targets]"
  echo ""
  echo "Examples:"
  echo ""
  echo "  # Build all CPU targets"
  echo "  $0"
  echo ""
  echo "  # Build and run Cluster Dynamics CLI with CUDA"
  echo "  $0 --cuda --run gpies"
  echo ""
  echo "Targets:"
  echo "  cd                  The Cluster Dynamics library"
  echo "  gpies               The CLI for the Cluster Dynamics library"
  echo "  gpiestests             GoogleTest based tests for Cluster Dynamics library"
  echo "  db                  The DB Library"
  echo "  dbcli               The CLI for the DB library"
  echo "  dbtests             GoogleTest based tests for DB library"
  echo "  okmc                The OKMC application"
  echo ""
  echo "Options:"
  echo "  --help              Show this help."
  echo "  --run, -r           Run target after build. Must be only one runnable target."
  echo "  --clean, -c         Clean everything before building the project."
  echo "                      Removes build, out, and db directories."
  echo "  --force, -f         Do not ask for confirmation when --clean is specified."
  echo "  --cpu               Build CPU targets."
  echo "                      This option is assumed if no --cuda specified."
  echo "  --cuda              Build CUDA targets for the current GPU architecture."
  echo "                      (nvcc -arch=native)"
  echo "  --cuda-all-major    Build CUDA targets for all major GPU architectures."
  echo "                      (nvcc -arch=all-major)"
  echo "  --debug             Build debug build (optimizations and sanitizer turned on)."
  echo "                      Cannot be usage together with --release."
  echo "  --release           Build release build (max optimizations)."
  echo "                      Cannot be usage together with --debug."
  echo "  --no-sanitizer      Do not use sanitizer for debug builds."
  echo "  --cmake-verbose     Enable verbose output in the build process."
  echo "                      (CMAKE_VERBOSE_MAKEFILE=ON)"
  exit $ERROR
fi

if [ "$DEBUG" -a "$RELEASE" ]; then
  echo_error "Both --debug and --release cannot be used at the same time."
fi

if [ ! "$CUDA" ]; then
  CPU=1
fi

for target in "${TARGETS[@]}"; do
  case $target in
    cd)
      CPU_TARGETS+=("clusterdynamics")
      CUDA_TARGETS+=("clusterdynamicscuda")
      ;;
    gpies)
      CPU_RUNNABLE_TARGETS+=("gpies")
      CUDA_RUNNABLE_TARGETS+=("gpies")
      ;;
    gpiestests)
      CPU_RUNNABLE_TARGETS+=("gpies_tests")
      CUDA_RUNNABLE_TARGETS+=("gpies_cuda_tests")
      ;;
    db)
      CPU_TARGETS+=("clientdb")
      ;;
    dbcli)
      CPU_RUNNABLE_TARGETS+=("db_cli")
      ;;
    dbtests)
      CPU_RUNNABLE_TARGETS+=("db_tests")
      ;;
    okmc)
      CPU_RUNNABLE_TARGETS+=("okmc")
      ;;
    *)
      UNKNOWN_TARGETS+=("$target")
      ;;
  esac
done

if [ "$CPU" ]; then
  TARGETS_TO_BUILD+=(${CPU_TARGETS[@]})
  TARGETS_TO_RUN+=(${CPU_RUNNABLE_TARGETS[@]})
fi

if [ "$CUDA" ]; then
  TARGETS_TO_BUILD+=(${CUDA_TARGETS[@]})
  TARGETS_TO_RUN+=(${CUDA_RUNNABLE_TARGETS[@]})
fi

TARGETS_TO_BUILD+=(${TARGETS_TO_RUN[@]})

if [ "${#UNKNOWN_TARGETS[@]}" -gt 0 ]; then
  error_text="Unknown targets: ${UNKNOWN_TARGETS[@]}"
  echo_error "$error_text"
elif [ "${#TARGETS[@]}" -gt 0 -a "${#TARGETS_TO_BUILD[@]}" -eq 0 ]; then
  echo_error "No targets to build. You might forgot to specify --cpu."
fi

if [ "$RUN" -a "${#TARGETS_TO_RUN[@]}" -gt 1 ]; then
  error_text="More than one target to run. Runnable targets: ${TARGETS_TO_RUN[@]}"
  echo_error "$error_text"
fi

if [ "$ERROR" ]; then
  exit $ERROR
fi

CMAKE_CONFIGURE_OPTIONS="-B .build -S ."
CMAKE_BUILD_OPTIONS="--build .build -j 4"

OUT_PATH="out"
if [ "$RELEASE" ]; then
  CMAKE_CONFIGURE_OPTIONS+=" -DCMAKE_BUILD_TYPE=Release"
  CMAKE_BUILD_OPTIONS+=" --config Release"
else
  CMAKE_CONFIGURE_OPTIONS+=" -DCMAKE_BUILD_TYPE=Debug"
  CMAKE_BUILD_OPTIONS+=" --config Debug"
fi

if [ "$CUDA" ]; then
  CMAKE_CONFIGURE_OPTIONS+=" -DGP_BUILD_CUDA:BOOL=true"
else
  CMAKE_CONFIGURE_OPTIONS+=" -DGP_BUILD_CUDA:BOOL=false"
fi

if [ "$CUDA_ALL" ]; then
  CMAKE_CONFIGURE_OPTIONS+=" -DCUDA_ARCHITECTURES=all-major"
else
  CMAKE_CONFIGURE_OPTIONS+=" -DCUDA_ARCHITECTURES=native"
fi

if [ "$NO_SANITIZER" ]; then
  CMAKE_CONFIGURE_OPTIONS+=" -DGP_NO_SANITIZER:BOOL=true"
else
  CMAKE_CONFIGURE_OPTIONS+=" -DGP_NO_SANITIZER:BOOL=false"
fi

if [ "$CMAKE_VERBOSE" ]; then
  CMAKE_CONFIGURE_OPTIONS+=" -DCMAKE_VERBOSE_MAKEFILE:BOOL=true"
else
  CMAKE_CONFIGURE_OPTIONS+=" -DCMAKE_VERBOSE_MAKEFILE:BOOL=false"
fi

for target in "${TARGETS_TO_BUILD[@]}"; do
  CMAKE_BUILD_OPTIONS+=" --target $target"
done

if [ "$CLEAN" ]; then
  if [ ! "$FORCE" ]; then
    echo "Are you sure you want to clean all build artifacts?"
    echo "This will delete the following folders: .build, out, db."
    read -p "Press Y to confirm: " -n 1 -r
    echo ""
    if [ "$REPLY" = "y" -o "$REPLY" = "Y" ]; then
      FORCE=1
    fi
  fi
  if [ "$FORCE" ]; then
    rm -rf .build out db
  else
    exit
  fi
fi

cmake $CMAKE_CONFIGURE_OPTIONS
cmake $CMAKE_BUILD_OPTIONS

if [ "$RUN" ]; then
  eval "./out/$TARGETS_TO_RUN $RUN_OPTIONS"
fi
