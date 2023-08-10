# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.
CXX ?= c++
CXXFLAGS ?= -g -Wall -O2


all:
	$(CXX) -o sort sort.cpp -std=c++17 -O3 -pthread -Wno-deprecated-declarations

debug:
	$(CXX) -o sort sort.cpp -std=c++17 -O3 -g -pthread -DDEBUG=1 -Wno-deprecated-declarations


# Usually there is no need to modify this
check: all
	$(MAKE) -C tests check
