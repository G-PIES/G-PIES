ifeq ($(OS), Windows_NT)
	TARGET_OS = windows
	CCFLAGS += -D WIN32
	CCFLAGS += -D_USE_MATH_DEFINES
	LIB_EXT := .lib
	EXE_EXT := .exe
	ifeq ($(PROCESSOR_ARCHITEW6432), AMD64)
		TARGET_ARCH = amd64
		CCFLAGS += -D AMD64
		GTEST_LIBS = ./extern/googletest/lib/windows
	else ifeq ($(PROCESSOR_ARCHITECTURE), AMD64)
		TARGET_ARCH = amd64
		CCFLAGS += -D AMD64
		GTEST_LIBS = ./extern/googletest/lib/windows
	else ifeq ($(PROCESSOR_ARCHITECTURE), x86)
		TARGET_ARCH = ia32
		CCFLAGS += -D IA32
		GTEST_LIBS = ./extern/googletest/lib/windows
	endif
else
	LIB_EXT := .a
	EXE_EXT := .out
	
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		TARGET_OS = linux
		CCFLAGS += -D LINUX
		GTEST_LIBS = ./extern/googletest/lib/linux
	else ifeq ($(UNAME_S), Darwin)
		TARGET_OS = macos
		CCFLAGS += -D OSX
		GTEST_LIBS = ./extern/googletest/lib/apple
	endif

	UNAME_M := $(shell uname -m)
	ifeq ($(UNAME_M), x86_64)
		TARGET_ARCH = amd64
		CCFLAGS += -D AMD64
	else ifneq ($(filter %86, $(UNAME_M)),)
		TARGET_ARCH = ia32
		CCFLAGS += -D IA32
	else ifneq ($(filter arm%, $(UNAME_M)),)
		TARGET_ARCH = arm
		CCFLAGS += -D ARM
	endif
endif

# ----------------------------------------------------------------------------------------
# Variables

CC = g++
CCFLAGS += -std=c++17 -fno-fast-math -Werror -Wall -Wextra
NVCCFLAGS += -std=c++17 -DUSE_CUDA -x cu -Werror all-warnings
CLANGFLAGS = $(CCFLAGS) -DUSE_METAL

INCLUDE_FLAGS = -Isrc/client_db -Isrc/cluster_dynamics -Isrc/cluster_dynamics/cpu -Isrc/cluster_dynamics/cuda -Isrc/cluster_dynamics/metal
INCLUDE_FLAGS += -Iinclude -Iinclude/utils

# Directories
BIN_DIR = bin
BUILD_DIR = build
DB_DIR = db
LIB_DIR = lib
SHADER_DIR = shader

# Libraries
CD_LIB = $(LIB_DIR)/libclusterdynamics$(LIB_EXT)
CDCUDA_LIB = $(LIB_DIR)/libclusterdynamicscuda$(LIB_EXT)
CDMETAL_LIB = $(LIB_DIR)/libclusterdynamicsmetal$(LIB_EXT)
DB_LIB = $(LIB_DIR)/libclientdb$(LIB_EXT)

# ----------------------------------------------------------------------------------------

.PHONY: bdirs clean lib cli cuda all cd cdlib cdcudalib dblib cdv cdcsv cdcudacli dbcli dbtests cluster_dynamics client_db

# ----------------------------------------------------------------------------------------
# Utilties 

# Setup Build Directories
bdirs:
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	mkdir -p $(DB_DIR)
	mkdir -p $(LIB_DIR)

# Clean Development Files
clean: 
	rm -fr $(BIN_DIR)
	rm -fr $(BUILD_DIR)
	rm -fr $(DB_DIR)/*.db
	rm -fr $(LIB_DIR)

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# Batch

lib: cdlib cdcudalib dblib

cli: cdcli dbcli

cuda: cdcudalib cdcudacli

all: lib cli cuda

cd: cdlib cdcudalib cdcli

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# Libraries 

cdmetallib: bdirs
	xcrun -sdk macosx metal -DGP_FLOAT=float $(SHADER_DIR)/metal/cluster_dynamics.metal -c -o $(BUILD_DIR)/cluster_dynamics.ir $(INCLUDE_FLAGS)
	xcrun -sdk macosx metallib -o $(LIB_DIR)/cluster_dynamics.metallib $(BUILD_DIR)/cluster_dynamics.ir
	$(CC) $(CLANGFLAGS) -DGP_FLOAT=float src/cluster_dynamics/cluster_dynamics.cpp -c -o $(BUILD_DIR)/clusterdynamicsmetal.o -Iextern/metal-cpp $(INCLUDE_FLAGS)
	$(CC) $(CLANGFLAGS) -DGP_FLOAT=float src/cluster_dynamics/material.cpp -c -o $(BUILD_DIR)/material.o $(INCLUDE_FLAGS)
	$(CC) $(CLANGFLAGS) -DGP_FLOAT=float src/cluster_dynamics/nuclear_reactor.cpp -c -o $(BUILD_DIR)/nuclear_reactor.o $(INCLUDE_FLAGS)
	$(CC) $(CLANGFLAGS) -DGP_FLOAT=float -DMETALLIB_PATH=\"${CURDIR}/$(LIB_DIR)/cluster_dynamics.metallib\" src/cluster_dynamics/metal/cluster_dynamics_metal_impl.cpp -c -o $(BUILD_DIR)/clusterdynamicsmetalimpl.o -Iextern/metal-cpp $(INCLUDE_FLAGS)
	$(CC) $(CLANGFLAGS) -DGP_FLOAT=float src/cluster_dynamics/metal/cluster_dynamics_metal_kernel.cpp -c -o $(BUILD_DIR)/clusterdynamicsmetalkernel.o -Iextern/metal-cpp $(INCLUDE_FLAGS)
	ar crs $(CDMETAL_LIB) $(BUILD_DIR)/clusterdynamicsmetal.o $(BUILD_DIR)/material.o $(BUILD_DIR)/nuclear_reactor.o $(BUILD_DIR)/clusterdynamicsmetalimpl.o $(BUILD_DIR)/clusterdynamicsmetalkernel.o

# Client Database CLI
dblib: bdirs
	$(CC) $(CCFLAGS) src/client_db/client_db.cpp -c -o $(BUILD_DIR)/clientdb.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/material.cpp -c -o $(BUILD_DIR)/material.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/nuclear_reactor.cpp -c -o $(BUILD_DIR)/nuclear_reactor.o $(INCLUDE_FLAGS)
	ar crs $(DB_LIB) $(BUILD_DIR)/material.o $(BUILD_DIR)/nuclear_reactor.o $(BUILD_DIR)/clientdb.o

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# Executables 
# NOTE: "make [target] R=1" will automatically run the executable following the build

# Cluster Dynamics W/ Metal CLI
cdmetalcli: cdmetallib
	$(CC) $(CCFLAGS) -DGP_FLOAT=float cli/cd_cli.cpp -o $(BIN_DIR)/cd_metal_cli$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamicsmetal -framework Metal -framework QuartzCore -framework Foundation
	@[ "${R}" ] && ./$(BIN_DIR)/cd_metal_cli$(EXE_EXT) || ( exit 0 )

# Database CLI
dbcli: dblib
	$(CC) $(CCFLAGS) cli/db_cli.cpp -o $(BIN_DIR)/db_cli$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclientdb -lsqlite3
	@[ "${R}" ] && ./$(BIN_DIR)/db_cli$(EXE_EXT) || ( exit 0 )

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# Tests 

# GoogleTest Database Unit Tests
dbtests: dblib
	$(CC) $(CCFLAGS) test/db_tests.cpp -o $(BIN_DIR)/db_tests$(EXE_EXT) $(INCLUDE_FLAGS) -I./extern/googletest/include -L$(GTEST_LIBS) -L$(LIB_DIR) -lgtest_main -lgtest -lpthread -lclientdb -lsqlite3
	@[ "${R}" ] && ./$(BIN_DIR)/db_tests$(EXE_EXT) || ( exit 0 )

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# CI Targets 

# Cluster Dynamics
cluster_dynamics: cdlib 
	$(CC) $(CCFLAGS) cli/cd_cli.cpp -o $(BIN_DIR)/cluster_dynamics$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamics

# Database 
client_db: dblib
	$(CC) $(CCFLAGS) cli/db_cli.cpp -o $(BIN_DIR)/client_db$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclientdb -lsqlite3

# ----------------------------------------------------------------------------------------

# New Makefile

.SECONDEXPANSION:

CONFIGURATION = release

# -----------------------------------------------------------------------------
# Meta targets and aliases

.PHONY: all_cpu all_cuda clean
all_cpu: okmc cd_cli cd_tests libclusterdynamics
all_cuda: cd_cuda_cli cdcuda_tests libclusterdynamicscuda

.PHONY: clean_build_dir
clean: clean_build_dir
clean_build_dir:
	rm -rf $(BUILD_DIR)

# Cluster Dynamics Library and tests
.PHONY: cdlib cdcudalib libclientdb cdtests cdcudatests
cdlib: libclusterdynamics
cdcudalib: libclusterdynamicscuda
clientdblib: libclientdb
cdtests: cd_tests
cdcudatests: cdcuda_tests

# Cluster Dynamics CLI
.PHONY: cdcli
cdcli: cd_cli

# Cluster Dynamics W/ CUDA CLI
.PHONY: cdcudacli
cdcli: cd_cuda_cli

# Cluster Dynamics CLI W/ Verbose Printing
.PHONY: cdv
cdv: cd_cli
cdv: CXXFLAGS += -D VPRINT=true -D VBREAK=true

# Cluster Dynamics CLI W/ CSV Output Formatting
.PHONY: cdcsv out_dir clean_out_dir
cdcsv: out_dir cd_cli
cdcsv: CXXFLAGS += -D CSV=true
cdcsv: RUN_ARGS = 1e-5 1 > out/cd-output.csv
out_dir:
	mkdir -p out
clean: clean_out_dir
clean_out_dir:
	rm -rf out

# -----------------------------------------------------------------------------
# Compiler parameters
#
# Supported variables
# - CXXFLAGS.{suffix}
# - CXXFLAGS.{compiler}.{suffix}
# Where suffix is one of the following
# - {configuration}
# - arch_{architecture}
# - os_{architecture}
#
# For example this flags will be used only for nvcc on Windows:
# CXXFLAGS.nvcc.os_windows = -example

COMPILER = gcc

INCLUDES = include

CXXFLAGS.common     = -MMD -MP -std=c++17 $(INCLUDES:%=-I%)
CXXFLAGS.release    = -O3
CXXFLAGS.debug      = -O0
CXXFLAGS.arch_amd64 = -DAMD64
CXXFLAGS.arch_ia32  = -DIA32
CXXFLAGS.arch_arm   = -DARM
CXXFLAGS.os_windows = -DWIN32 -D_USE_MATH_DEFINES
CXXFLAGS.os_linux   = -DLINUX
CXXFLAGS.os_macos   = -DOSX

CXX.gcc = g++
CXXFLAGS.gcc.common = -Wall -fno-fast-math
CXXFLAGS.gcc.debug  = -g3 -fsanitize=undefined -fsanitize=address

CXX.nvcc = nvcc
CXXFLAGS.nvcc.common = -Werror all-warnings -DUSE_CUDA -x cu --expt-extended-lambda

calculate_compiler_options = $(strip $($1.common) $($1.$(CONFIGURATION)) $($1.arch_$(TARGET_ARCH)) $($1.os_$(TARGET_OS)))
CXX      = $(CXX.$(COMPILER))
CXXFLAGS = $(strip $(call calculate_compiler_options,CXXFLAGS) $(call calculate_compiler_options,CXXFLAGS.$(COMPILER)))

# -----------------------------------------------------------------------------
# Linker parameters

LIBRARIES             =
EXTERN_LIBRARIES      =
EXTERN_LIBRARIES_PATH =

LDFLAGS.common  = -L$(BUILD_PATH) $(LIBRARIES:%=-l%) $(EXTERN_LIBRARIES_PATH:%=-L%) $(EXTERN_LIBRARIES:%=-l%)

LDFLAGS.gcc.debug   = -fsanitize=undefined -fsanitize=address

LD.gcc = g++
LD.nvcc = nvcc

LD = $(LD.$(COMPILER))
LDFLAGS = $(strip $(call calculate_compiler_options,LDFLAGS) $(call calculate_compiler_options,LDFLAGS.$(COMPILER)))

# -----------------------------------------------------------------------------
# Arciver parameters
AR = ar
ARFLAGS = -crvs

# -----------------------------------------------------------------------------
# Generic source and artifact paths
BUILD_PATH    = build/$(CONFIGURATION)
OBJ_PATH      = $(BUILD_PATH)/obj
get_obj_files = $(addprefix $(OBJ_PATH)/$1/,$(CXX_FILES.$1:.cpp=.o))

EXE_EXT.os_linux   =
EXE_EXT.os_macos   =
EXE_EXT.os_windows = .exe
get_exe_file       = $(BUILD_PATH)/$1$(EXE_EXT.os_$(TARGET_OS))

LIB_EXT.os_linux   = .a
LIB_EXT.os_macos   = .a
LIB_EXT.os_windows = .lib
get_lib_file       = $(BUILD_PATH)/$1$(LIB_EXT.os_$(TARGET_OS))

# Recursive wildcard function
# Source: https://stackoverflow.com/a/18258352
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# -----------------------------------------------------------------------------
# Cluster Dynamics Library
ALL_LIB += libclusterdynamics
CXX_FILES.libclusterdynamics = $(wildcard src/cluster_dynamics/*.cpp) $(call rwildcard,src/cluster_dynamics/cpu,*.cpp)
OBJ_FILES.libclusterdynamics = $(call get_obj_files,libclusterdynamics)
$(OBJ_FILES.libclusterdynamics): INCLUDES += src/cluster_dynamics src/cluster_dynamics/cpu

# -----------------------------------------------------------------------------
# Cluster Dynamics CUDA Library
ALL_LIB += libclusterdynamicscuda
CXX_FILES.libclusterdynamicscuda = $(wildcard src/cluster_dynamics/*.cpp) $(call rwildcard,src/cluster_dynamics/cuda,*.cpp)
OBJ_FILES.libclusterdynamicscuda = $(call get_obj_files,libclusterdynamicscuda)
$(OBJ_FILES.libclusterdynamicscuda): COMPILER = nvcc
$(OBJ_FILES.libclusterdynamicscuda): INCLUDES += src/cluster_dynamics src/cluster_dynamics/cuda

# -----------------------------------------------------------------------------
# Client Database Library 
ALL_LIB += libclientdb
CXX_FILES.libclientdb = $(wildcard src/client_db/*.cpp)
OBJ_FILES.libclientdb = $(call get_obj_files,libclientdb)
$(OBJ_FILES.libclientdb): INCLUDES += include/utils
$(OBJ_FILES.libclientdb): EXTERN_LIBRARIES += sqlite3

# -----------------------------------------------------------------------------
# GoogleTest Cluster Dynamics Unit Tests
ALL_EXE += cd_tests
CXX_FILES.cd_tests = test/cd_tests.cpp
OBJ_FILES.cd_tests = $(call get_obj_files,cd_tests)
EXE_FILE.cd_tests = $(call get_exe_file,cd_tests)
$(OBJ_FILES.cd_tests): INCLUDES += src/cluster_dynamics src/cluster_dynamics/cpu extern/googletest/include
$(EXE_FILE.cd_tests): LIBRARIES += clusterdynamics
$(EXE_FILE.cd_tests): EXTERN_LIBRARIES += gtest gtest_main pthread
$(EXE_FILE.cd_tests): EXTERN_LIBRARIES_PATH += extern/googletest/lib/$(TARGET_OS)

# -----------------------------------------------------------------------------
# GoogleTest Cluster Dynamics W/ CUDA Unit Tests
ALL_EXE += cdcuda_tests
CXX_FILES.cdcuda_tests = test/cd_tests.cpp
OBJ_FILES.cdcuda_tests = $(call get_obj_files,cdcuda_tests)
EXE_FILE.cdcuda_tests = $(call get_exe_file,cdcuda_tests)
$(OBJ_FILES.cdcuda_tests): COMPILER = nvcc
$(OBJ_FILES.cdcuda_tests): INCLUDES += src/cluster_dynamics src/cluster_dynamics/cuda extern/googletest/include
$(EXE_FILE.cdcuda_tests): COMPILER = nvcc
$(EXE_FILE.cdcuda_tests): LIBRARIES += clusterdynamicscuda
$(EXE_FILE.cdcuda_tests): EXTERN_LIBRARIES += gtest gtest_main pthread
$(EXE_FILE.cdcuda_tests): EXTERN_LIBRARIES_PATH += extern/googletest/lib/$(TARGET_OS)

# -----------------------------------------------------------------------------
# Cluster Dynamics CLI
ALL_EXE += cd_cli
CXX_FILES.cd_cli = cli/cd_cli.cpp
OBJ_FILES.cd_cli = $(call get_obj_files,cd_cli)
EXE_FILE.cd_cli = $(call get_exe_file,cd_cli)
$(EXE_FILE.cd_cli): LIBRARIES += clusterdynamics clientdb
$(EXE_FILE.cd_cli): EXTERN_LIBRARIES += sqlite3

# Cluster Dynamics W/ CUDA CLI
ALL_EXE += cd_cuda_cli
CXX_FILES.cd_cuda_cli = cli/cd_cli.cpp
OBJ_FILES.cd_cuda_cli = $(call get_obj_files,cd_cuda_cli)
EXE_FILE.cd_cuda_cli = $(call get_exe_file,cd_cuda_cli)
$(OBJ_FILES.cd_cuda_cli): COMPILER = nvcc
$(EXE_FILE.cd_cuda_cli): COMPILER = nvcc
$(EXE_FILE.cd_cuda_cli): LIBRARIES += clusterdynamicscuda

# -----------------------------------------------------------------------------
# OKMC
ALL_EXE += okmc
CXX_FILES.okmc = $(call rwildcard,src/okmc,*.cpp)
OBJ_FILES.okmc = $(call get_obj_files,okmc)

# -----------------------------------------------------------------------------
# Generic C++ compile target
define CXX_template
$$(OBJ_PATH)/$1/%.o: %.cpp
	@mkdir -p $$(@D)
	$$(CXX) $$(CXXFLAGS) -c $$< -o $$@
endef

$(foreach prog,$(ALL_EXE) $(ALL_LIB),$(eval $(call CXX_template,$(prog))))
-include $(call rwildcard,$(OBJ_PATH)*.d)

# -----------------------------------------------------------------------------
# Generic executable target
.PHONY: $(ALL_EXE)
all: $(ALL_EXE)

ifeq ($(R), 1)
$(ALL_EXE): %: $(BUILD_PATH)/%$(EXE_EXT.os_$(TARGET_OS))
	@[ "$(R)" ] && $< $(RUN_ARGS) || ( exit 0 )
else
$(ALL_EXE): %: $(BUILD_PATH)/%$(EXE_EXT.os_$(TARGET_OS))
endif

ALL_EXE_FILES = $(foreach exe,$(ALL_EXE),$(call get_exe_file,$(exe)))
$(ALL_EXE_FILES): $(call get_exe_file,%): $$(foreach lib,$$(LIBRARIES),$$(call get_lib_file,lib$$(lib))) $$(OBJ_FILES.%)
	$(LD) $(filter %.o,$^) $(LDFLAGS) -o $@

# -----------------------------------------------------------------------------
# Generic library target
ALL_LIB_FILES = $(foreach lib,$(ALL_LIB),$(call get_lib_file,$(lib)))
.PHONY: $(ALL_LIB)
all: $(ALL_LIB)
$(ALL_LIB): %: $(BUILD_PATH)/%$(LIB_EXT.os_$(TARGET_OS))
$(ALL_LIB_FILES): $(call get_lib_file,%): $$(OBJ_FILES.%)
	$(AR) $(ARFLAGS) $@ $(filter %.o,$^)
