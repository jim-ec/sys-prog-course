CC ?= cc
CFLAGS ?= -g -Wall -O2
CXX ?= c++
CXXFLAGS ?= -g -Wall -O2


all: librw_1.so librw_2.so tracer

librw_1.so: task-1_1.c
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $<

librw_2.so: task-1_2.c
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $<

tracer: task-1_3.c
	$(CC) $(CFLAGS) -o $@ $<

check: all
	$(MAKE) -C tests check
