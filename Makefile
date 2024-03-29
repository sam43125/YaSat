# A template C++ Makefile for your SAT solver.

# Debugging flags
#FLAGS=-Wall -Wold-style-cast -Wformat=2 -ansi -pedantic -ggdb3 -DDEBUG

# Optimizing flags
FLAGS=-Wall -Wold-style-cast -Wformat=2 -ansi -pedantic -O3

# List all the .o files you need to build here
OBJS=parser.o sat.o solver.o

# This is the name of the executable file that gets built.  Please
# don't change it.
EXENAME=yasat

# Compile targets
all: $(OBJS)
	g++ $(FLAGS) $(OBJS) -lz -o $(EXENAME)
parser.o: parser.cpp parser.h
	g++ $(FLAGS) -c parser.cpp
sat.o: sat.cpp parser.h
	g++ $(FLAGS) -std=c++17 -c sat.cpp
solver.o: solver.cpp solver.hpp
	g++ $(FLAGS) -std=c++17 -c solver.cpp
# Add more compilation targets here



# The "phony" `clean' compilation target.  Type `make clean' to remove
# your object files and your executable.
.PHONY: clean
clean:
	rm -rf $(OBJS) $(EXENAME)
