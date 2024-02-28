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
INCLUDE_FLAGS += -Iinclude

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

.PHONY: bdirs clean lib ex cuda all cd cdlib cdcudalib dblib cdv cdcsv cdcudaex dbex dbtests cluster_dynamics client_db

# ----------------------------------------------------------------------------------------
# Utilties 

# Setup Build Directories
bdirs:
	$(call mkdir, $(BIN_DIR))
	$(call mkdir, $(BUILD_DIR))
	$(call mkdir, $(DB_DIR))
	$(call mkdir, $(LIB_DIR))

# Clean Development Files
clean: 
	$(call rmdir,$(BIN_DIR))
	$(call rmdir,$(BUILD_DIR))
	$(call rmdir,$(DB_DIR))
	$(call rmdir,$(LIB_DIR))

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# Batch

lib: cdlib cdcudalib dblib

ex: cdex dbex

cuda: cdcudalib cdcudaex

all: lib ex cuda

cd: cdlib cdcudalib cdex

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
	ar crs $(CDMETAL_LIB) $(BUILD_DIR)/clusterdynamicsmetal.o $(BUILD_DIR)/material.o $(BUILD_DIR)/nuclear_reactor.o $(BUILD_DIR)/clusterdynamicsmetalimpl.o 

# Client Database Example
dblib: bdirs
	$(CC) $(CCFLAGS) src/client_db/client_db.cpp -c -o $(BUILD_DIR)/clientdb.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/material.cpp -c -o $(BUILD_DIR)/material.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/nuclear_reactor.cpp -c -o $(BUILD_DIR)/nuclear_reactor.o $(INCLUDE_FLAGS)
	ar crs $(DB_LIB) $(BUILD_DIR)/material.o $(BUILD_DIR)/nuclear_reactor.o $(BUILD_DIR)/clientdb.o

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# Executables 
# NOTE: "make [target] R=1" will automatically run the executable following the build

# Cluster Dynamics W/ Metal Example
cdmetalex: cdmetallib
	$(CC) $(CCFLAGS) -DGP_FLOAT=float example/cd_example.cpp -o $(BIN_DIR)/cd_metal_example$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamicsmetal -framework Metal -framework QuartzCore -framework Foundation
	@[ "${R}" ] && ./$(BIN_DIR)/cd_metal_example$(EXE_EXT) || ( exit 0 )

# Database Example
dbex: dblib
	$(CC) $(CCFLAGS) example/db_example.cpp -o $(BIN_DIR)/db_example$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclientdb -lsqlite3
	@[ "${R}" ] && ./$(BIN_DIR)/db_example$(EXE_EXT) || ( exit 0 )

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
	$(CC) $(CCFLAGS) example/cd_example.cpp -o $(BIN_DIR)/cluster_dynamics$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamics

# Database 
client_db: dblib
	$(CC) $(CCFLAGS) example/db_example.cpp -o $(BIN_DIR)/client_db$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclientdb -lsqlite3

# ----------------------------------------------------------------------------------------

# New Makefile

.SECONDEXPANSION:

CONFIGURATION = release

# -----------------------------------------------------------------------------
# Meta targets and aliases

.PHONY: all_cpu all_cuda clean
all_cpu: okmc cd_example cd_tests libclusterdynamics
all_cuda: cd_cuda_example cdcuda_tests libclusterdynamicscuda

.PHONY: clean_build_dir
clean: clean_build_dir
clean_build_dir:
	$(call rmdir,$(BUILD_DIR))

# Cluster Dynamics Library and tests
.PHONY: cdlib cdcudalib cdtests cdcudatests
cdlib: libclusterdynamics
cdcudalib: libclusterdynamicscuda
cdtests: cd_tests
cdcudatests: cdcuda_tests

# Cluster Dynamics Example
.PHONY: cdex
cdex: cd_example

# Cluster Dynamics W/ CUDA Example
.PHONY: cdcudaex
cdcudaex: cd_cuda_example

# Cluster Dynamics Example W/ Verbose Printing
.PHONY: cdv
cdv: cd_example
cdv: CXXFLAGS += -D VPRINT=true -D VBREAK=true

# Cluster Dynamics Example W/ CSV Output Formatting
.PHONY: cdcsv out_dir clean_out_dir
cdcsv: out_dir cd_example
cdcsv: CXXFLAGS += -D CSV=true
cdcsv: RUN_ARGS = 1e-5 1 > out/cd-output.csv
out_dir:
	@$(call mkdir,out)
clean: clean_out_dir
clean_out_dir:
	$(call rmdir,out)

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

CXXFLAGS.common     = $(INCLUDES:%=-I%)
CXXFLAGS.arch_amd64 = -DAMD64
CXXFLAGS.arch_ia32  = -DIA32
CXXFLAGS.arch_arm   = -DARM
CXXFLAGS.os_windows = -DWIN32 -D_USE_MATH_DEFINES
CXXFLAGS.os_linux   = -DLINUX
CXXFLAGS.os_macos   = -DOSX

CXX.gcc = g++
CXXFLAGS.gcc.common  = -std=c++20 -Wall -fno-fast-math -MMD -MP
CXXFLAGS.gcc.release = -O3
CXXFLAGS.gcc.debug   = -O0 -g3
CXXFLAGS.gcc.debug.os_linux = -fsanitize=undefined -fsanitize=address
CXXFLAGS.gcc.debug.os_macos = -fsanitize=undefined -fsanitize=address

CXX.nvcc = nvcc
CXXFLAGS.nvcc.common = -Werror all-warnings -DUSE_CUDA -x cu --expt-extended-lambda
CXXFLAGS.nvcc.release = -O3
CXXFLAGS.nvcc.debug   = -O0
CXXFLAGS.nvcc.os_windows = -MD
CXXFLAGS.nvcc.os_linux   = -MMD -MP
CXXFLAGS.nvcc.os_macos   = -MMD -MP

ifeq ($(TARGET_OS), windows)
COMPILER = cl
SHELL = cmd

VSWHERE = C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe
VC_BASE_PATH = $(shell $(VSWHERE) -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)
VC_VERSION = $(shell cmd /c type "$(VC_BASE_PATH)\VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt")
VC_CL_PATH = $(VC_BASE_PATH)\VC\Tools\MSVC\$(VC_VERSION)

reg_query = $(shell cmd /q /c for /F "tokens=1,2*" %%i in ('reg query "$1" /v $2') do (if "%%i"=="$2" echo %%k))
WIN_SDK_VERSION = v10.0
WIN_SDK_BASE_PATH       = $(call reg_query,HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\$(WIN_SDK_VERSION),InstallationFolder)
WIN_SDK_PRODUCT_VERSION = $(call reg_query,HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\$(WIN_SDK_VERSION),ProductVersion).0

CXX.cl = "$(VC_CL_PATH)\bin\Hostx64\x64\cl.exe"
CXXFLAGS.cl.common = -nologo -std:c++20 \
                     -I"$(VC_CL_PATH)\include" \
                     -I"$(WIN_SDK_BASE_PATH)\Include\$(WIN_SDK_PRODUCT_VERSION)\ucrt"
CXXFLAGS.cl.release = -O2
CXXFLAGS.cl.debug   = -Od

LD.cl = "$(VC_CL_PATH)\bin\Hostx64\x64\link.exe"
LDFLAGS.cl.common = -nologo \
                    -libpath:$(BUILD_PATH) $(EXTERN_LIBRARIES_PATH:%=-libpath:%) \
					-libpath:"$(VC_CL_PATH)\lib\x64" \
					-libpath:"$(WIN_SDK_BASE_PATH)\lib\$(WIN_SDK_PRODUCT_VERSION)\ucrt\x64" \
					-libpath:"$(WIN_SDK_BASE_PATH)\lib\$(WIN_SDK_PRODUCT_VERSION)\um\x64" \
					-libpath:"$(VC_CL_PATH)\lib\x64" \
					$(LIBRARIES:%=%.lib) $(EXTERN_LIBRARIES:%=%.lib)

AR.os_windows = "$(VC_CL_PATH)\bin\Hostx64\x64\lib.exe"
ARFLAGS.os_windows = -nologo

compile.cl = $(CXX) $(CXXFLAGS) -Fo$(dir $2) -c $1
link.cl = $(LD) $(LDFLAGS) -out:$2 $1
archive.os_windows = $(AR) -out:$2 $1
endif

calculate_compiler_options = $(strip $($1.common) $($1.$(CONFIGURATION)) \
							 $($1.arch_$(TARGET_ARCH)) $($1.$(CONFIGURATION).arch_$(TARGET_ARCH)) \
							 $($1.os_$(TARGET_OS)) $($1.$(CONFIGURATION).os_$(TARGET_OS)))
CXX      = $(CXX.$(COMPILER))
CXXFLAGS = $(strip $(call calculate_compiler_options,CXXFLAGS) $(call calculate_compiler_options,CXXFLAGS.$(COMPILER)))
compile  = $(if $(compile.$(COMPILER)),$(compile.$(COMPILER)),$(CXX) $(CXXFLAGS) -c $1 -o $2)

# -----------------------------------------------------------------------------
# Linker parameters

LIBRARIES             =
EXTERN_LIBRARIES      =
EXTERN_LIBRARIES_PATH =

LDFLAGS.gcc.common  = -L$(BUILD_PATH) $(LIBRARIES:%=-l%) $(EXTERN_LIBRARIES_PATH:%=-L%) $(EXTERN_LIBRARIES:%=-l%)
LDFLAGS.nvcc.common = -L$(BUILD_PATH) $(LIBRARIES:%=-l%) $(EXTERN_LIBRARIES_PATH:%=-L%) $(EXTERN_LIBRARIES:%=-l%)

LDFLAGS.gcc.debug.os_linux = -fsanitize=undefined -fsanitize=address
LDFLAGS.gcc.debug.os_macos = -fsanitize=undefined -fsanitize=address

LD.gcc = g++
LD.nvcc = nvcc

LD = $(LD.$(COMPILER))
LDFLAGS = $(strip $(call calculate_compiler_options,LDFLAGS) $(call calculate_compiler_options,LDFLAGS.$(COMPILER)))
link = $(if $(link.$(COMPILER)),$(link.$(COMPILER)),$(LD) $(filter %$(OBJ_EXT.os_$(TARGET_OS)),$^) $(LDFLAGS) -o $2)

# -----------------------------------------------------------------------------
# Archiver parameters
AR.os_linux = ar
AR.os_macos = ar
ARFLAGS.os_linux = -crvs
ARFLAGS.os_macos = -crvs
AR      = $(AR.os_$(TARGET_OS))
ARFLAGS = $(ARFLAGS.os_$(TARGET_OS))
archive = $(if $(archive.os_$(TARGET_OS)),$(archive.os_$(TARGET_OS)),$(AR) $(ARFLAGS) $2 $1)

# -----------------------------------------------------------------------------
# Generic source and artifact paths
BUILD_PATH    = build/$(CONFIGURATION)
OBJ_PATH      = $(BUILD_PATH)/obj
OBJ_EXT.os_linux   = .o
OBJ_EXT.os_macos   = .o
OBJ_EXT.os_windows = .obj
get_obj_files = $(addprefix $(OBJ_PATH)/$1/,$(CXX_FILES.$1:.cpp=$(OBJ_EXT.os_$(TARGET_OS))))

EXE_EXT.os_linux   =
EXE_EXT.os_macos   =
EXE_EXT.os_windows = .exe
get_exe_file       = $(BUILD_PATH)/$1$(EXE_EXT.os_$(TARGET_OS))

LIB_EXT.os_linux        = .a
LIB_EXT.os_macos        = .a
LIB_EXT.os_windows      = .lib
get_lib_file.os_linux   = $(BUILD_PATH)/$1$(LIB_EXT.os_$(TARGET_OS))
get_lib_file.os_macos   = $(get_lib_file.os_linux)
get_lib_file.os_windows = $(BUILD_PATH)/$(1:lib%=%)$(LIB_EXT.os_$(TARGET_OS))
get_lib_file            = $(get_lib_file.os_$(TARGET_OS))

mkdir.os_linux   = mkdir -p $1
mkdir.os_macos   = $(mkdir.os_linux)
mkdir.os_windows = cmd /c if not exist $(subst /,\,$1) mkdir $(subst /,\,$1)
mkdir            = $(mkdir.os_$(TARGET_OS))

rmdir.os_linux   = rm -rf $1
rmdir.os_macos   = $(rmdir.os_linux)
rmdir.os_windows = cmd /c if exist $(subst /,\,$1) rmdir /s /q $(subst /,\,$1)
rmdir            = $(rmdir.os_$(TARGET_OS))

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
# GoogleTest Cluster Dynamics Unit Tests
ALL_EXE += cd_tests
CXX_FILES.cd_tests = test/cd_tests.cpp
OBJ_FILES.cd_tests = $(call get_obj_files,cd_tests)
EXE_FILE.cd_tests = $(call get_exe_file,cd_tests)
$(OBJ_FILES.cd_tests): INCLUDES += src/cluster_dynamics src/cluster_dynamics/cpu extern/googletest/include
$(EXE_FILE.cd_tests): LIBRARIES += clusterdynamics
$(EXE_FILE.cd_tests): EXTERN_LIBRARIES += gtest gtest_main
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
$(EXE_FILE.cdcuda_tests): EXTERN_LIBRARIES += gtest gtest_main
$(EXE_FILE.cdcuda_tests): EXTERN_LIBRARIES_PATH += extern/googletest/lib/$(TARGET_OS)

# -----------------------------------------------------------------------------
# Cluster Dynamics Example
ALL_EXE += cd_example
CXX_FILES.cd_example = example/cd_example.cpp
OBJ_FILES.cd_example = $(call get_obj_files,cd_example)
EXE_FILE.cd_example = $(call get_exe_file,cd_example)
$(EXE_FILE.cd_example): LIBRARIES += clusterdynamics

# Cluster Dynamics W/ CUDA Example
ALL_EXE += cd_cuda_example
CXX_FILES.cd_cuda_example = example/cd_example.cpp
OBJ_FILES.cd_cuda_example = $(call get_obj_files,cd_cuda_example)
EXE_FILE.cd_cuda_example = $(call get_exe_file,cd_cuda_example)
$(OBJ_FILES.cd_cuda_example): COMPILER = nvcc
$(EXE_FILE.cd_cuda_example): COMPILER = nvcc
$(EXE_FILE.cd_cuda_example): LIBRARIES += clusterdynamicscuda

# -----------------------------------------------------------------------------
# OKMC
ALL_EXE += okmc
CXX_FILES.okmc = $(call rwildcard,src/okmc,*.cpp)
OBJ_FILES.okmc = $(call get_obj_files,okmc)

# -----------------------------------------------------------------------------
# Generic C++ compile target
define CXX_template
$$(OBJ_PATH)/$1/%$(OBJ_EXT.os_$(TARGET_OS)): %.cpp
	@$$(call mkdir,$$(@D))
	$$(call compile,$$<,$$@)
endef

$(foreach prog,$(ALL_EXE) $(ALL_LIB),$(eval $(call CXX_template,$(prog))))
-include $(call rwildcard,$(OBJ_PATH)*.d)

# -----------------------------------------------------------------------------
# Generic executable target
.PHONY: $(ALL_EXE)

ifeq ($(R), 1)
$(ALL_EXE): %: $(call get_exe_file,%)
	@[ "$(R)" ] && $< $(RUN_ARGS) || ( exit 0 )
else
$(ALL_EXE): %: $(call get_exe_file,%)
endif

ALL_EXE_FILES = $(foreach exe,$(ALL_EXE),$(call get_exe_file,$(exe)))
$(ALL_EXE_FILES): $(call get_exe_file,%): $$(foreach lib,$$(LIBRARIES),$$(call get_lib_file,lib$$(lib))) $$(OBJ_FILES.%)
	$(call link,$^,$@)

# -----------------------------------------------------------------------------
# Generic library target
.PHONY: $(ALL_LIB)

define LIB_template
$1: $(call get_lib_file,$1)
$(call get_lib_file,$1): $$(OBJ_FILES.$1)
	$$(call archive,$$^,$$@)
endef

$(foreach lib,$(ALL_LIB),$(eval $(call LIB_template,$(lib))))
