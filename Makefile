ifeq ($(OS), Windows_NT)
	CCFLAGS += -D WIN32
	CCFLAGS += -D_USE_MATH_DEFINES
	LIB_EXT := .lib
	EXE_EXT := .exe
	ifeq ($(PROCESSOR_ARCHITEW6432), AMD64)
		CCFLAGS += -D AMD64
	else ifeq ($(PROCESSOR_ARCHITECTURE), AMD64)
		CCFLAGS += -D AMD64
	else ifeq ($(PROCESSOR_ARCHITECTURE), x86)
		CCFLAGS += -D IA32
	endif
else
	LIB_EXT := .a
	EXE_EXT := .out
	
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		CCFLAGS += -D LINUX
		GTEST_LIBS = ./extern/googletest/lib/linux
	else ifeq ($(UNAME_S), Darwin)
		CCFLAGS += -D OSX
		GTEST_LIBS = ./extern/googletest/lib/apple
	endif

	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P), x86_64)
		CCFLAGS += -D AMD64
	else ifneq ($(filter %86, $(UNAME_P)),)
		CCFLAGS += -D IA32
	else ifneq ($(filter arm%, $(UNAME_P)),)
		CCFLAGS += -D ARM
	endif
endif


# ----------------------------------------------------------------------------------------
# Variables

CC = g++
CCFLAGS += -std=c++17

INCLUDE_FLAGS = -Isrc/client_db -Isrc/cluster_dynamics
INCLUDE_FLAGS += -Iinclude/client_db -Iinclude/cluster_dynamics -Iinclude/model -Iinclude/utils

# Directories
BIN_DIR = bin
BUILD_DIR = build
DB_DIR = db
LIB_DIR = lib
OUT_DIR = out

# Libraries
CD_LIB = $(LIB_DIR)/libclusterdynamics$(LIB_EXT)
CDCUDA_LIB = $(LIB_DIR)/libclusterdynamicscuda$(LIB_EXT)
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
	$(CC) $(CCFLAGS) src/cluster_dynamics/cpu/*.cpp -c -o $(BUILD_DIR)/clusterdynamicsimpl.o $(INCLUDE_FLAGS)
	ar crs $(CD_LIB) $(BUILD_DIR)/clusterdynamics.o $(BUILD_DIR)/clusterdynamicsimpl.o

# Cluster Dynamics CUDA Library
cdcudalib: bdirs
	nvcc -O3 $(CCFLAGS) -DUSE_CUDA src/cluster_dynamics/cluster_dynamics.cpp -c -o $(BUILD_DIR)/clusterdynamicscuda.o $(INCLUDE_FLAGS)
	nvcc -c -x cu --expt-extended-lambda -DUSE_CUDA $(CCFLAGS) src/cluster_dynamics/cuda/*.cpp -o $(BUILD_DIR)/clusterdynamicscudaimpl.o $(INCLUDE_FLAGS)
	ar crs $(CDCUDA_LIB) $(BUILD_DIR)/clusterdynamicscuda.o $(BUILD_DIR)/clusterdynamicscudaimpl.o

# Client Database Example
dblib: bdirs
	$(CC) $(CCFLAGS) src/client_db/*.cpp -c -o $(BUILD_DIR)/clientdb.o $(INCLUDE_FLAGS)
	ar crs $(DB_LIB) $(BUILD_DIR)/clientdb.o

# ----------------------------------------------------------------------------------------


# ----------------------------------------------------------------------------------------
# Executables 
# NOTE: "make [target] R=1" will automatically run the executable following the build

# Cluster Dynamics W/ CERN ROOT UI 
# -- you need to have cern root installed locally for the libraries to link correctly.
cldui: rootui/ui_main.cpp $(library)
	$(CC) $(CCFLAGS) rootui/ui_main.cpp $$(root-config --glibs --cflags --libs) -o $(BIN_DIR)/cd_ui$(EXE_EXT) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics

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
	nvcc example/cd_example.cpp -o $(BIN_DIR)/cd_cuda_example$(EXE_EXT) $(INCLUDE_FLAGS) -L$(LIB_DIR) -lclusterdynamicscuda
	@[ "${R}" ] && ./$(BIN_DIR)/cd_cuda_example$(EXE_EXT) || ( exit 0 )

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
	nvcc $(CCFLAGS) -x cu -DUSE_CUDA test/cd_tests.cpp -o $(BIN_DIR)/cdcuda_tests$(EXE_EXT) $(INCLUDE_FLAGS) -I./extern/googletest/include -L$(GTEST_LIBS) -L$(LIB_DIR) -lgtest_main -lgtest -lpthread -lclusterdynamicscuda
	@[ "${R}" ] && ./$(BIN_DIR)/cdcuda_tests$(EXE_EXT) || ( exit 0 )

# GoogleTest Cluster Dynamics Unit Tests
dbtests:
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