cc	= gcc
cxx	= g++
cxxflags	= -g -std=c++11 -Wall

all:
	$(cxx) $(cxxflags) shell.cpp -o simple_shell
.PHONY: clean
clean:
	rm -f *.o
