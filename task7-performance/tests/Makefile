# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.

.PHONY: check

all: | build/Makefile
	$(MAKE) compile


compile:
	$(MAKE) -C build db_bench


build/Makefile: | build
	cd build \
	&& cmake ../rocksdb/ -DFAIL_ON_WARNINGS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLS1_QUIET=1


build:
	mkdir -p $@


optimized_run: | build/Makefile
	$(MAKE) -C build db_bench
	touch $@


clean:
	-rm -rf build
	-rm cache-optimized


cache-optimized: optimized_run
	./build/db_bench -flagfile config --num=50000 --threads=16 | tee cache-optimized


cache-run: cache-optimized
	$(MAKE) -C .. all
	../build/db_bench -flagfile config --num=50000 --threads=16 | tee cache-run


check: cache-run cache-optimized
	-python3 test.py check 0
	-rm cache-run

