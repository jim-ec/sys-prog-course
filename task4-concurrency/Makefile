# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.
CC ?= cc
CFLAGS ?= -g -Wall -O2
CXX ?= c++
CXXFLAGS ?= -g -Wall -O2
CARGO ?= cargo
RUSTFLAGS ?= -g

.PHONY: all clean check

# C/C++ example
all: libcspinlock.so liblockhashmap.so liblockfreehashmap.so

libcspinlock.so: src/spinlock.cpp
	$(CXX) $(CFLAGS) -std=c++17 -shared -fPIC -ldl -I . -o $@ $<

liblockhashmap.so: src/hashmap.cpp
	$(CXX) $(CFLAGS) -std=c++17 -shared -fPIC -ldl -I . -o $@ $<

liblockfreehashmap.so: src/lockfreehashmap.cpp
	$(CXX) $(CFLAGS) -std=c++17 -shared -fPIC -ldl -I . -o $@ $<

# Usually there is no need to modify this
check: all
	$(MAKE) -C tests check

clean:
	$(MAKE) -C tests clean
	rm -rf *.so* *.o
