CC = g++
NVCC = nvcc

CCFLAGS += -std=c++17
INCLUDE_DIR = ./include
LIB_DIR = ./lib

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
	else ifeq ($(UNAME_S), Darwin)
		CCFLAGS += -D OSX
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

binary = cluster_dynamics$(EXE_EXT)
library = lib/libclusterdynamics$(LIB_EXT)

#----------------------------------------------------------------------------------------
# Targets
#----------------------------------------------------------------------------------------

all: software_lib software_example_frontend

.PHONY: software_lib software_example_frontend cuda_example_frontend clean

# software library compilation
software_lib:
	mkdir -p lib
	$(CC) $(CCFLAGS) src/cluster_dynamics.cpp -c -o libclusterdynamics.o -I$(INCLUDE_DIR) -I./src
	ar crs $(library) libclusterdynamics.o
	rm libclusterdynamics.o

# CUDA library compilation
cuda_lib:
	mkdir -p lib
	nvcc -O3 -c -x cu -DUSE_CUDA $(CCFLAGS) src/cluster_dynamics.cpp -o libclusterdynamics.o -I$(INCLUDE_DIR) -I./src
	ar crs $(library) libclusterdynamics.o
	rm libclusterdynamics.o

# example frontend compilation
example_frontend: software_lib
	$(CC) example/*.cpp -o $(binary) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics 

# CUDA backend & example frontend compilation
cuda_example_frontend: cuda_lib
	nvcc example/*.cpp -o $(binary) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics

# compile and run example frontend
cdr: example/main.cpp $(library)
	$(CC) $(CCFLAGS) example/*.cpp -o $(binary) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics
	./$(binary)

# example frontend w/ debug symbols
debug:
	$(CC) $(CCFLAGS) -g example/*.cpp -o $(binary) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics

# example frontend w/ verbose printing and debug symbols
vprint:
	$(CC) $(CCFLAGS) -g -D VPRINT=true -D VBREAK=true example/*.cpp -o $(binary) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics

# example frontend w/ verbose printing, debug symbols, and run on compilation
vprintr:
	$(CC) $(CCFLAGS) -g -D VPRINT=true -D VBREAK=true example/*.cpp -o $(binary) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics
	./$(binary)

# build example frontend, then run and export results to cd-output.csv
csv:
	$(CC) $(CCFLAGS) -D CSV=true example/*.cpp -o $(binary) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics
	./$(binary) 1e-5 1 > cd-output.csv

# run the example frontend
run:
	./$(binary)

# remove binaries
clean:
	rm *$(ext) lib/*$(LIB_EXT)
