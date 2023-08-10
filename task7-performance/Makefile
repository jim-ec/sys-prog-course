# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.

.PHONY: all compile check

NUM?=50000
#THREADS?=$(shell ./cpu.sh | grep "logical" | awk '{print $$NF}')
THREADS?=16

# this target should build all executables for all tests
all: | build/Makefile
	$(MAKE) compile


compile:
	$(MAKE) -C build db_bench


build/Makefile: | build
	cd build \
	&& cmake ../rocksdb/ -DFAIL_ON_WARNINGS=OFF -DCMAKE_BUILD_TYPE=Release -DLS1_QUIET=1


build:
	mkdir -p $@


run_small: NUM=10000
run_small: run

run_large: NUM=500000
run_large: run

run: all
	./build/db_bench -flagfile tests/config --num=$(NUM) --threads=$(THREADS)


clean:
	-rm -rf build


check: all
	$(MAKE) -C tests check
