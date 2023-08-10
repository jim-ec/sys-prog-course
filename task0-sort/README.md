# Implement sort(1)

The goal of this assigment is to get familiar with the task format and
your chosen system programming language (C, C++ or Rust). 
It will be not part of your final grade.
Once your repository is created from the general template,
the continous integration services will build your project and will run
the tests on your programs (see the Actions tab on github).

## The build system

Each assignment comes with a template [Makefile](Makefile) as the make build system that
needs to be adapted depending on the programming language.
All assignments will try to build the `all` target within the Makefile like this:

```console
$ make all
```

So make sure your that the `all` target will produce all executables required for the
tests. 

The github build environment comes with all tools for building C, C++ and Rust pre-installed.
At the time of writing the following set up is installed:

- C/C++ compilers: gcc (different versions from 7. to 10 i.e. gcc-10), clang 6-9
- C/C++ build systems: cmake: 3.19.6 autoconf: 2.69, automake: 1.15.1
- Rust compiler/build system: rustc / cargo: 1.51.0

## Tests

Our tests will lookup exectuables to be in one of the following directories (assuming `./` is the project root):

- `./`
- `./target/release`
- `./target/debug`

where the latter two directories are usually used by Rust's build system
[cargo](https://doc.rust-lang.org/cargo/index.html).

After that it runs individual tests coming from the `tests/` folder (test
scripts are prefixed with `test_`).
Each test program is a python3 script and can be run individually, i.e.:

```console
python3 ./tests/test_sort.py
```

For convenience our Makefile also comes with `check` target which will run all tests in serial:

```console
$ make check
```

- All tasks use https://github.com/ls1-sys-prog-course-internal/task-template as a template

For the rare occassions that bugs experienced in the online test system but not
locally, it is also possible to run the github action environment locally with
using [act](https://github.com/nektos/act) using the `Large docker image`.


## The assignment for this week

1. Your task is it to write a program that reads lines from standard input (also known as stdin)
and prints all lines sorted to standard output (also known as stdout) in ascending order.
For simplicty all test inputs can be assumed
[ASCII](https://en.wikipedia.org/wiki/ASCII) encoded and the comparison is done
byte-wise. The program will be called like this:

``` console
./sort < input-file
```

2. Further more your program should accept a flag as the first argument on command line `-r` which
will reverse the output

``` console
./sort -r < input-file
```

3. Make sure your program can also sort its input using a fixed amount of memory.
We will test your program by applying by `ulimit -v 131072` in its parent shell,
which will limit the program to 128MB:

``` console
bash -c 'ulimit -v 131072; ./sort < input-file'
```

*Hint:* This is commonly known as external sort.

### 1. Test: tests/test_sort.py

TODO

### 2. Test: tests/test_sort_reverse.py

TODO

### 3. Test: tests/test_sort_limited_memory.py

TODO
