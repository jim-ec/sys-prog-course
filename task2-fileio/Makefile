# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.
CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic 

all:
	$(CXX) $(CXXFLAGS) -o memfs memfs.cpp `pkg-config fuse --cflags --libs`

check: all
	$(MAKE) -C tests check
