# Implement file system operations using fuse

The goal of this assigment is to implement a filesystem using [fuse](https://en.wikipedia.org/wiki/Filesystem_in_Userspace). Fuse allows one
to create their own file system without editing kernel code. On successful
completion of this excercise you will have a good understanding of filesystem
data structures and how filesystem operations are mapped onto these data structures.
You will create the filesystem in a system programming language (C, C++ or Rust) of your choice. 
This assignment will be graded for a total of **30 points**.

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
- fuse: 2.9.9-3

## Tests

Our tests will lookup exectuables in one of the following directories (assuming `./` is the project root):

- `./`
- `./target/release`
- `./target/debug`

where the latter two directories are usually used by Rust's build system
[cargo](https://doc.rust-lang.org/cargo/index.html).

After that it runs individual tests coming from the `tests/` folder (test
scripts are prefixed with `test_`).
Each test program is a python3 script and can be run individually, i.e.:

```console
python3 ./tests/test_mount.py
```

For convenience our Makefile also comes with `check` target which will run all tests in serial:

```console
$ make check
```

- All tasks use https://github.com/ls1-sys-prog-course-internal/task-template as a template

For the rare occassion that bugs are experienced in the CI but not
locally, it is also possible to run the github action environment locally
with [docker](https://www.docker.com/) using this [container

``` console
# This will mount your current directory as /code into the container
 docker run -ti --entrypoint=/bin/bash -v $(pwd):/code --rm ghcr.io/ls1-sys-prog-course-internal/ls1-runner:latest
```

## The assignment for this week

Your task is to implement an **in-memory** filesystem, by implementing the filesystem's data structures and
file operations that correctly manipulate them. Please note that you will be graded against the correctness of your implementation only. We do not check for performance. However, it is a good practice to figure out what data structures are necessary to efficiently support the respective file operations. The filesystem should be implemented with the following constraints:
- Maximum file name length of 255 ascii characters.
- Maximum file size of 512 bytes.

### Deliverables

1. A mountable filesystem with the correct file type (directory). It should be possible to mount the filesystem as follows:

``` console
./memfs [mount point]
```

Where the argument "mount point" is the directory where the filesystem will be mounted at.

2. The ability to create flat files and directories in the in-memory filesystem i.e, all files and directories stored in a single directory which is the root of the filesystem.

3. The ability to create hierarchical files and directories in the in-memory filesystem i.e, create files and directories in directories inside the root directory of the filesystem.

4. The ability to write data to and read data from files.

5. The ability to append data to an existing file.

You are free to choose any data structure to manage filesystem data and metadata. To ensure that the filesystem has the above functionality the following functions must be implemented from the fuse API:
- lookup (mandatory if you implement based on a low level interface with inodes)
- getattr
- read
- write
- readdir
- mkdir
- mknod
- open
- create

## Going further

If you want to go further, you can implement the remaining file operations from the fuse API. You could also investigate other data structures found in modern filesystems (like BTrees), to try and improve the performance of your filesystem. Note that this will not be considered towards the final grade.

The current, fuse filesystem, was implemented entirely in userspace. You could also look into how file systems are implemented in the kernel. For example you could build the very simple filesystem that was described in the lecture, to be run from within the kernel. One reference implementation of such a filesystem can be found [here](https://github.com/rgouicem/ouichefs).
