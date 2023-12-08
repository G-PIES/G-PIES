ifeq ($(OS), Windows_NT)
	CC = g++
	CCFLAGS += -D WIN32
	ifeq ($(PROCESSOR_ARCHITEW6432), AMD64)
		CCFLAGS += -D AMD64
	else ifeq ($(PROCESSOR_ARCHITECTURE), AMD64)
		CCFLAGS += -D AMD64
	else ifeq ($(PROCESSOR_ARCHITECTURE), x86)
		CCFLAGS += -D IA32
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		CC = g++
		CCFLAGS += -D LINUX
	else ifeq ($(UNAME_S), Darwin)
		CC = clang
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

CCFLAGS += -std=c++17
INCLUDE_DIR = ./include
LIB_DIR = ./lib

ext = .out
binary = cluster_dynamics$(ext)
library = lib/libclusterdynamics.so

ifdef C
	CCFLAGS += -D CONCENTRATION_BOUNDARY=$(C)
endif

ifdef T
	CCFLAGS += -D SIMULATION_TIME=$(T)
endif

ifdef DT
	CCFLAGS += -D DELTA_TIME=$(DT)
endif

ifdef N
	CCFLAGS += -D N=$(N)
endif

.PHONY: lib cuda cluster_dynamics

# library compilation
lib: src/cluster_dynamics.cpp 
	$(CC) $(CCFLAGS) src/*.cpp -shared -fPIC -c -o $(library) -I$(INCLUDE_DIR) -I./src

cuda: src/cluster_dynamics.cu
	nvcc -O3 src/cluster_dynamics.cu -c -I$(INCLUDE_DIR) -o cudacd.o
	nvcc -O3 $(CCFLAGS) src/cluster_dynamics.cpp -c -o lib.o -I$(INCLUDE_DIR) -I./src
	nvcc -O3 example/main.cpp -c -o main.o -I$(INCLUDE_DIR) -L$(LIB_DIR)
	nvcc -O3 main.o lib.o cudacd.o -o cluster_dynamics.out

# example frontend compilation
cluster_dynamics: example/main.cpp $(library)
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
	rm *$(ext) lib/*.so