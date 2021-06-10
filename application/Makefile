# A template C++ Makefile for your SAT solver.

# Debugging flags
#FLAGS=-Wall -Wold-style-cast -Wformat=2 -ansi -pedantic -ggdb3 -DDEBUG

# Optimizing flags
FLAGS=-Wall -Wold-style-cast -Wformat=2 -ansi -pedantic -O3

# List all the .o files you need to build here
OBJS=solver.o n_queen.o

# This is the name of the executable file that gets built.  Please
# don't change it.
EXENAME=n_queen

# Compile targets
all: $(OBJS)
	g++ $(FLAGS) $(OBJS) -lz -o $(EXENAME)
solver.o:
	g++ $(FLAGS) -std=c++17 -c ../solver.cpp
n_queen.o: n_queen.cpp
	g++ $(FLAGS) -std=c++17 -c n_queen.cpp
# Add more compilation targets here



# The "phony" `clean' compilation target.  Type `make clean' to remove
# your object files and your executable.
.PHONY: clean
clean:
	rm -rf $(OBJS) $(EXENAME)
