# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.
CC ?= cc
CFLAGS ?= -g -Wall -O2
CXX ?= c++
CXXFLAGS ?= -g -Wall -O2
CARGO ?= cargo
RUSTFLAGS ?= -g
LDFLAGS ?= -pthread
# CPPFLAGS += -DYYDEBUG -DDEBUG

include c.make

check: all
	$(MAKE) -C tests check
