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
INCLUDE_DIRS = ./include

ext = .out
binary = cluster_dynamics$(ext)

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

# standard compilation
cluster_dynamics: src/main.cpp
	$(CC) $(CCFLAGS) src/*.cpp -o $(binary) -I$(INCLUDE_DIRS)

# compile and run
cdr: main.cpp
	$(CC) $(CCFLAGS) src/*.cpp -o $(binary) -I$(INCLUDE_DIRS)
	./$(binary)

# debug symbols
debug:
	$(CC) $(CCFLAGS) -g src/*.cpp -o $(binary) -I$(INCLUDE_DIRS)

# verbose printing and debug symbols
vprint:
	$(CC) $(CCFLAGS) -g -D VPRINT=true -D VBREAK=true src/*.cpp -o $(binary) -I$(INCLUDE_DIRS)

# verbose printing, debug symbols, and run on compilation
vprintr:
	$(CC) $(CCFLAGS) -g -D VPRINT=true -D VBREAK=true src/*.cpp -o $(binary) -I$(INCLUDE_DIRS)
	./$(binary)

csv:
	$(CC) $(CCFLAGS) -D CSV=true src/*.cpp -o $(binary) -I$(INCLUDE_DIRS)
	./$(binary) > cd-output.csv

# run the binary
run:
	./$(binary)

# remove binaries
clean:
	rm *$(ext)