# Performance

This weeks exercise requires you to optimize [RocksDB](https://rocksdb.org/). RocksDB is a high-performance persistent LSM-based key value store ([pdf](https://www.cs.umb.edu/~poneil/lsmtree.pdf)). We introduced changes to make the performance of rocksdb worse in throughput and latency.

Your task is to get as close to unmodified RocksDB performance as possible with the rocksdb version in the directory `rocksdb`, by finding the changes and fixing them.

## Deliveries

The execution of the benchmark `db_bench` has to be correct and as fast as possible.


## General Information

0. After you cloned the repo please execute once:
   ```console
   $ sed -i 's/ubuntu-latest/self-hosted/g' .github/workflows/classroom.yml
   $ git add .github/workflows/classroom.yml
   $ git commit -m "fixed classroom.yml"
   ```
   This makes sure that the tests will run on our internal runners.

1. For building run 
   ```console
   $ make all
   # or for faster build time
   $ make all -j$(./cpu.sh | grep "logical" | awk '{print $NF}')
   ```
2. For running `db_bench` run:
   ```console
   $ make run
   ```
3. For running the tests run:
   ```console
   $ make check
   ```
4. Except the `check` run all other configuration take a `NUM` and `THREADS` parameter to configure the tests to your system e.g.:
   ```console
   $ make run NUM=10000 THREADS=4
   ```
   However, the grading test will use `NUM=50000` and `THREADS=16`
   
 ## Requirements
 1. tee
 2. python3
 3. gcc
 4. cmake
 5. gflags
   
 ## Tips
 You are encouraged to use [perf](http://www.brendangregg.com/perf.html) and [flamegraph](http://www.brendangregg.com/flamegraphs.html) to profile the application and identify the parts of the code that might slowdown performance. You can either adopt the `Makefile` or copy the run command from the `Makefile` to run `db_bench` with perf.
 Perf and flamegraphs also support different modes, some of them might be useful.
 
 The changes we introduced should be obvious, even if you do not know C++. Thus no major changes to the source code should be required.
 
 Especially, before you made the first few optimization, you probably want to lower the number of `NUM`, as it could be painfully slow otherwise. However, later you should increase the number to be equal to the grading test as otherwise you might not benchmark/profile all source paths.
 
 ## References
 [RocksDB](https://rocksdb.org/)  
 [LSM Datastructure](https://www.cs.umb.edu/~poneil/lsmtree.pdf)  
 [Perf](http://www.brendangregg.com/perf.html)  
 [Flamegraph](http://www.brendangregg.com/flamegraphs.html)  
 [Intel VTunes](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/vtune-profiler.html#gs.3hg9lw)  
 [AMD μProf](https://developer.amd.com/amd-uprof/)  
 [Gperftools](https://gperftools.github.io/gperftools/)  
 [heaptrack](https://github.com/KDE/heaptrack)  
 [Valgrind](https://valgrind.org/)  
 [Google Orbit](https://github.com/google/orbit)  
 [GNU gprof](https://sourceware.org/binutils/docs/gprof/)  
 [Google Benchmark](https://github.com/google/benchmark)  
 [QuickBench](https://quick-bench.com/)  
 [CMake](https://cmake.org/)  
 [CppRefence](https://en.cppreference.com/w/)  
 [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)  
