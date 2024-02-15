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

	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P), x86_64)
		TARGET_ARCH = amd64
		CCFLAGS += -D AMD64
	else ifneq ($(filter %86, $(UNAME_P)),)
		TARGET_ARCH = ia32
		CCFLAGS += -D IA32
	else ifneq ($(filter arm%, $(UNAME_P)),)
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
INCLUDE_FLAGS += -Iinclude/client_db -Iinclude/cluster_dynamics -Iinclude/model -Iinclude/utils

# Directories
BIN_DIR = bin
BUILD_DIR = build
DB_DIR = db
LIB_DIR = lib
OUT_DIR = out
SHADER_DIR = shader

# Libraries
CD_LIB = $(LIB_DIR)/libclusterdynamics$(LIB_EXT)
CDCUDA_LIB = $(LIB_DIR)/libclusterdynamicscuda$(LIB_EXT)
CDMETAL_LIB = $(LIB_DIR)/libclusterdynamicsmetal$(LIB_EXT)
DB_LIB = $(LIB_DIR)/libclientdb$(LIB_EXT)

# ----------------------------------------------------------------------------------------

.PHONY: bdirs clean lib ex cuda all cd cdlib cdcudalib dblib cdex cdv cdcsv cdcudaex dbex cdtests dbtests cluster_dynamics client_db

# ----------------------------------------------------------------------------------------
# Utilties 

# Setup Build Directories
bdirs:
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	mkdir -p $(DB_DIR)
	mkdir -p $(LIB_DIR)
	mkdir -p $(OUT_DIR)

# Clean Development Files
clean: 
	rm -fr $(BIN_DIR)
	rm -fr $(BUILD_DIR)
	rm -fr $(DB_DIR)/*.db
	rm -fr $(LIB_DIR)
	rm -fr $(OUT_DIR)

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

# Cluster Dynamics Library
cdlib: bdirs
	$(CC) $(CCFLAGS) src/cluster_dynamics/cluster_dynamics.cpp -c -o $(BUILD_DIR)/clusterdynamics.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/material.cpp -c -o $(BUILD_DIR)/material.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/nuclear_reactor.cpp -c -o $(BUILD_DIR)/nuclear_reactor.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/cpu/*.cpp -c -o $(BUILD_DIR)/clusterdynamicsimpl.o $(INCLUDE_FLAGS)
	ar crs $(CD_LIB) $(BUILD_DIR)/clusterdynamics.o $(BUILD_DIR)/material.o $(BUILD_DIR)/nuclear_reactor.o $(BUILD_DIR)/clusterdynamicsimpl.o

# Cluster Dynamics CUDA Library
cdcudalib: bdirs
	nvcc $(NVCCFLAGS) src/cluster_dynamics/cluster_dynamics.cpp -c -o $(BUILD_DIR)/clusterdynamicscuda.o $(INCLUDE_FLAGS)
	nvcc $(NVCCFLAGS) src/cluster_dynamics/material.cpp -c -o $(BUILD_DIR)/material.o $(INCLUDE_FLAGS)
	nvcc $(NVCCFLAGS) src/cluster_dynamics/nuclear_reactor.cpp -c -o $(BUILD_DIR)/nuclear_reactor.o $(INCLUDE_FLAGS)
	nvcc $(NVCCFLAGS) -c --expt-extended-lambda  src/cluster_dynamics/cuda/*.cpp -o $(BUILD_DIR)/clusterdynamicscudaimpl.o $(INCLUDE_FLAGS)
	ar crs $(CDCUDA_LIB) $(BUILD_DIR)/clusterdynamicscuda.o $(BUILD_DIR)/material.o $(BUILD_DIR)/nuclear_reactor.o $(BUILD_DIR)/clusterdynamicscudaimpl.o

cdmetallib: bdirs
	xcrun -sdk macosx metal -DGP_FLOAT=float $(SHADER_DIR)/metal/cluster_dynamics.metal -c -o $(BUILD_DIR)/cluster_dynamics.ir $(INCLUDE_FLAGS)
	xcrun -sdk macosx metallib -o $(LIB_DIR)/cluster_dynamics.metallib $(BUILD_DIR)/cluster_dynamics.ir
	clang++ $(CLANGFLAGS) -DGP_FLOAT=float src/cluster_dynamics/*.cpp -c -o $(BUILD_DIR)/clusterdynamicsmetal.o -Iextern/metal-cpp $(INCLUDE_FLAGS)
	clang++ $(CLANGFLAGS) -DGP_FLOAT=float -DMETALLIB_PATH=\"${CURDIR}/$(LIB_DIR)/cluster_dynamics.metallib\" src/cluster_dynamics/metal/*.cpp -c -o $(BUILD_DIR)/clusterdynamicsmetalimpl.o -Iextern/metal-cpp $(INCLUDE_FLAGS)
	ar crs $(CDMETAL_LIB) $(BUILD_DIR)/clusterdynamicsmetal.o $(BUILD_DIR)/clusterdynamicsmetalimpl.o

# Client Database Example
dblib: bdirs
	$(CC) $(CCFLAGS) src/client_db/*.cpp -c -o $(BUILD_DIR)/clientdb.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/material.cpp -c -o $(BUILD_DIR)/material.o $(INCLUDE_FLAGS)
	$(CC) $(CCFLAGS) src/cluster_dynamics/nuclear_reactor.cpp -c -o $(BUILD_DIR)/nuclear_reactor.o $(INCLUDE_FLAGS)
	ar crs $(DB_LIB) $(BUILD_DIR)/material.o $(BUILD_DIR)/nuclear_reactor.o $(BUILD_DIR)/clientdb.o

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# Executables 
# NOTE: "make [target] R=1" will automatically run the executable following the build

# Cluster Dynamics Example
cdex: cdlib
	$(CC) $(CCFLAGS) example/cd_example.cpp -o $(BIN_DIR)/cd_example$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamics
	@[ "${R}" ] && ./$(BIN_DIR)/cd_example$(EXE_EXT) || ( exit 0 )

# Cluster Dynamics Example W/ Verbose Printing
cdv:
	$(CC) $(CCFLAGS) -g -D VPRINT=true -D VBREAK=true example/cd_example.cpp -o $(BIN_DIR)/cd_example$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamics
	@[ "${R}" ] && ./$(BIN_DIR)/cd_example$(EXE_EXT) || ( exit 0 )

# Cluster Dynamics Example W/ CSV Output Formatting
cdcsv: cdlib
	$(CC) $(CCFLAGS) -D CSV=true example/cd_example.cpp -o $(BIN_DIR)/cd_example$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamics
	@[ "${R}" ] && ./$(BIN_DIR)/cd_example$(EXE_EXT) 1e-5 1 > $(OUT_DIR)/cd-output.csv || ( exit 0 )

# Cluster Dynamics W/ CUDA Example
cdcudaex: cdcudalib
	nvcc $(NVCCFLAGS) example/cd_example.cpp -o $(BIN_DIR)/cd_cuda_example$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamicscuda
	@[ "${R}" ] && ./$(BIN_DIR)/cd_cuda_example$(EXE_EXT) || ( exit 0 )

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

# GoogleTest Cluster Dynamics Unit Tests
cdtests: cdlib
	$(CC) $(CCFLAGS) test/cd_tests.cpp -o $(BIN_DIR)/cd_tests$(EXE_EXT) $(INCLUDE_FLAGS) -I./extern/googletest/include -L$(GTEST_LIBS) -L$(LIB_DIR) -lgtest_main -lgtest -lpthread -lclusterdynamics
	@[ "${R}" ] && ./$(BIN_DIR)/cd_tests$(EXE_EXT) || ( exit 0 )

# GoogleTest Cluster Dynamics W/ CUDA Unit Tests
cdcudatests: cdcudalib
	nvcc $(NVCCFLAGS) test/cd_tests.cpp -o $(BIN_DIR)/cdcuda_tests$(EXE_EXT) $(INCLUDE_FLAGS) -I./extern/googletest/include -L$(GTEST_LIBS) -L$(LIB_DIR) -lgtest_main -lgtest -lpthread -lclusterdynamicscuda
	@[ "${R}" ] && ./$(BIN_DIR)/cdcuda_tests$(EXE_EXT) || ( exit 0 )

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

# Compiler parameters
CXX = g++
CXXFLAGS.common     = -MMD -MP -Wall -std=c++17 -fno-fast-math
CXXFLAGS.release    = -O3
CXXFLAGS.debug      = -O0 -g3 -fsanitize=undefined -fsanitize=address
CXXFLAGS.arch_amd64 = -D AMD64
CXXFLAGS.arch_ia32  = -D IA32
CXXFLAGS.arch_arm   = -D ARM
CXXFLAGS.os_windows = -D WIN32 -D_USE_MATH_DEFINES
CXXFLAGS.os_linux   = -D LINUX
CXXFLAGS.os_maxos   = -D OSX

INCLUDES = src/client_db src/cluster_dynamics include/client_db include/cluster_dynamics include/model include/utils

CXXFLAGS = $(strip $(CXXFLAGS.common) $(CXXFLAGS.$(CONFIGURATION)) $(CXXFLAGS.arch_$(TARGET_ARCH)) $(CXXFLAGS.os_$(TARGET_OS)) $(INCLUDES:%=-I%))

# Linker parameters
LD = g++
LDFLAGS.common  =
LDFLAGS.release =
LDFLAGS.debug   =

LDFLAGS  = $(strip $(LDFLAGS.common) $(LDFLAGS.$(CONFIGURATION)))

# Generic source and artifact paths
SRC_PATH      = ./src
EXAMPLE_PATH  = ./example
BUILD_PATH    = ./build/$(CONFIGURATION)
OBJ_PATH      = $(BUILD_PATH)/obj
get_obj_name  = $(patsubst $(EXAMPLE_PATH)/%.cpp,$(OBJ_PATH)/example/%.o,$(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$1))

# Component-specific object files
CLIENT_DB_OBJ_FILES  = $(call get_obj_name,$(sort $(shell find $(SRC_PATH)/client_db -type f -name '*.cpp')))
CD_BASE_OBJ_FILES    = $(call get_obj_name,$(SRC_PATH)/cluster_dynamics/*.cpp)
CD_CPU_OBJ_FILES     = $(call get_obj_name,$(sort $(shell find $(SRC_PATH)/cluster_dynamics/cpu -type f -name '*.cpp')))
CD_CUDA_OBJ_FILES    = $(call get_obj_name,$(sort $(shell find $(SRC_PATH)/cluster_dynamics/cuda -type f -name '*.cpp')))
OKMC_OBJ_FILES       = $(call get_obj_name,$(sort $(shell find $(SRC_PATH)/okmc -type f -name '*.cpp')))
CD_EXAMPLE_OBJ_FILES = $(call get_obj_name,$(EXAMPLE_PATH)/cd_example.cpp)
DB_EXAMPLE_OBJ_FILES = $(call get_obj_name,$(EXAMPLE_PATH)/db_example.cpp)

# OKMC
ALL_EXE += okmc
EXE_okmc_PREREQUISITES = $(OKMC_OBJ_FILES)

# Generic C++ compile target
ALL_CXX_FILES := $(foreach path,$(SRC_PATH) $(EXAMPLE_PATH),$(sort $(shell find $(path) -type f -name '*.cpp')))
ALL_OBJ_FILES := $(call get_obj_name,$(ALL_CXX_FILES))
ALL_DEP_FILES := $(ALL_OBJ_FILES:%.o,%.d)
-include $(ALL_DEP_FILES)

$(ALL_OBJ_FILES): $(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generic executable target
EXE_EXT.os_linux   =
EXE_EXT.os_macos   =
EXE_EXT.os_windows = .exe

ALL_EXE_FILES = $(ALL_EXE:%=$(BUILD_PATH)/%$(EXE_EXT.os_$(TARGET_OS)))
.PHONY: $(ALL_EXE)
$(ALL_EXE): %: $(BUILD_PATH)/%$(EXE_EXT.os_$(TARGET_OS))
$(ALL_EXE_FILES): $(BUILD_PATH)/%$(EXE_EXT.os_$(TARGET_OS)): $$(EXE_%_PREREQUISITES)
	$(LD) $(LDFLAGS) $^ -o $@
