# Task 1 - Kernel and System Calls

## Task 1.1

Write a wrapper for the `read` and `write` system calls by using the `syscall` function from the glibc. Add the following optimsation: if the number of bytes to read/write is 0, do not call the system call.

The resulting binary should be a shared library named `librw_1.so` and should be usable by any application to replace the original glibc implementations of these two wrappers without changing the behaviour of applications.

## Task 1.2

Reuse your wrappers from task 1.1 and replace the `syscall` function with assembly that performs the system call. You must manage error reporting in the same way the glibc would.

The resulting binary should be a shared library named `librw_2.so` and should be usable by any application to replace the original glibc implementations of these two wrappers without changing the behaviour of applications.

### Inline assembly
*For C/C++ users:* You can use `asm()` ([docs](https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html)).

*For Rust users:* You can use `asm!()` from rust nightly ([docs](https://doc.rust-lang.org/nightly/unstable-book/library-features/asm.html)).

Our CI system also has rust nightly installed. It is accessible via `cargo +nightly build`.

## Task 1.3

Write a program that traces the system calls performed by another program.
To do this, you will need to use the `ptrace()` system call to attach a tracer process to a traced process (*tracee*).
Your program should take the program to trace and its arguments as arguments.
If you want to trace the `ls -l` program, you should run:

```console
$ ./tracer ls -l
```

Your tracing program should output to stderr in the following format:
```console
syscall_name(arg1, arg2, ...) = retval
```
Arguments should be correctly formatted, in a similar fashion to what `strace` does.
For example, when a `read` system call is detected on file descriptor 3, with a buffer address of 0x7fffa4398078, and a size of 832 bytes, the tracer should output:
```console
read(3, 0x7fffa4398078, 832) = 832
```

For this task, you only need to support tracing two system calls: `read` and `write`.
You are free to support other system calls if you want to.

You should carefully read the manpage of the `ptrace` system call to understand how to use it.
You can also find various resources online on how to use `ptrace()`.

The resulting binary should be an executable named `tracer`.

*Note*: To understand what is available in the registers that you can get through the `ptrace` system call, we strongly advise you to use a debugger such as `gdb`.

## Going further (not graded)

If you want to go further, you can try to implement a new system call in the Linux kernel. You can find a lot of documentation on the internet, as well as examples in the Linux kernel source code (you can use this [indexing website](https://elixir.bootlin.com/linux/latest/source) to easily navigate in kernel code).

## General information

1. Edit Makefile to include the build command used for your language
2. For building run 
   ```console
   $ make all
   ```
3. For running the tests run:
   ```console
   $ make check
   ```
