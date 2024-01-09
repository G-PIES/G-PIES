CC = g++
NVCC = nvcc

CCFLAGS += -std=c++17
INCLUDE_DIR = ./include
LIB_DIR = ./lib

ifeq ($(OS), Windows_NT)
	CCFLAGS += -D WIN32
	CCFLAGS += -D_USE_MATH_DEFINES
	LIB_EXT := .dll
	EXE_EXT := .exe
	ifeq ($(PROCESSOR_ARCHITEW6432), AMD64)
		CCFLAGS += -D AMD64
	else ifeq ($(PROCESSOR_ARCHITECTURE), AMD64)
		CCFLAGS += -D AMD64
	else ifeq ($(PROCESSOR_ARCHITECTURE), x86)
		CCFLAGS += -D IA32
	endif
else
	LIB_EXT := .so
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

all: software_lib cuda_frontend

.PHONY: lib cuda cluster_dynamics clean

# library compilation
software_lib: src/cluster_dynamics.cu
	mkdir -p lib
	$(CC) $(CCFLAGS) src/cluster_dynamics.cu -shared -fPIC -c -o $(library) -I$(INCLUDE_DIR) -I./src

# CUDA backend & example frontend compilation
cuda_frontend: src/cluster_dynamics.cu
	nvcc -O3 -pg -D USE_CUDA $(CCFLAGS) src/cluster_dynamics.cu -c -o lib.o -I$(INCLUDE_DIR) -I./src
	nvcc -O3 -pg example/main.cpp -c -o main.o -I$(INCLUDE_DIR) -L$(LIB_DIR)
	nvcc -O3 -pg main.o lib.o -o cluster_dynamics.out
	rm *.o

# example frontend compilation
example_frontend: example/main.cpp $(library)
	$(CC) example/*.cpp -o $(binary) -I$(INCLUDE_DIR) -L$(LIB_DIR) -lclusterdynamics

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
