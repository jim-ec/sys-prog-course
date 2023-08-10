# Implement functionalities of a shell

The goal of this assignment is to implement an interactive shell.  To simplify
the task you will be given a parser based on flex and bison that parses
expressions like:

``` console
ls -l
ls -l > /tmp/files
grep foo < /tmp/files
ls -l | wc -l
sleep 100 &
```


into a data structure `struct pipeline` that consists of one or more commands.
i.e.

``` console
ls -la | wc -l > /tmp/lines &
```

would be parsed as:

```c
struct pipeline = {
  .first_command = {
    .next = {
      .argv = {"wc", "-l", NULL},
      .argv_cap = 8,
      .argc = 2,
      .output_redir = "/tmp/lines",
      .input_redir = NULL,
    }
    .argv = {"ls", "-la", NULL},
    .argv_cap = 8,
    .argc = 2,
    .output_redir = NULL,
    .input_redir = NULL,
  };
  .background = 1,
}
```

See `parse.h` for further documentation. Furthermore, the shell parser also
recognizes the following builtins: `wait`, `exit`, `kill`. Those will be
parsed into `enum builtin_type`, while an optional argument goes into
`buitin_arg`.

Your task is to implement two functions:

## run_pipeline

`run_pipeline` should execute one pipeline structure. If a pipeline references
more than one `struct command` then each command stdin should be connected to the
stdout of the previous command with a pipe (except for the first command, which
has no previous stdin it could be connected to).
If `output_redir` is set then the commands stdout should be redirected to file
path specified. Stdout should be writeable.
If `input_redir` is set then the commands stdin should be redirected to the file
path specified. Stdin should be readable.

## run_builtin

`run_builtin` should perform one of the three builtin types:

`wait [PID]`: Accepts an optional argument `PID` via builtin_arg (builtin_arg is
NULL if not provided by the user). If a pid is provided the shell should use
`waitpid(2)` on this pid.  If no pid is provided it should use the
`waitpid(2)` to wait for any child process.

`exit [CODE]`: Accepts an optional argument `CODE` via builtin_arg (builtin_arg
is NULL if not provided by the user). If an exitcode is provided the shell
should use `exit(2)` to terminate the shell with the given exitcode. If no
`CODE` argument is provided the shell should exit with exit code 0.

`kill PID`: Accepts an argument `PID` via builtin_arg (builtin_arg is NULL if not provided by the user). 
If the argument is not provided (`builtin_arg` is NULL) then a meaningfull error message should be printed.
When the kill builtin is called, the shell should send a SIGTERM signal to the process specified by pid.


## Test setup

The test setup expects an executable `shell` in the current directory.

The parser given requires flex and bison to be installed, i.e.

``` console
$ apt install flex bison
```

For C/C++ uncomment `c.make` in the Makefile and implement your functionality in `execute.c`.

For Rust uncomment `rust.make` and implement functions in `src/lib.rs`. This
will compile a crate as a static library and link this library against the
`shell` executable. Since this is a ffi binding, you will need unsafe constructs
to access `struct pipeline` (see https://doc.rust-lang.org/book/ch19-01-unsafe-rust.html).
For rust use `fork` and `execve` from the `libc` crate or `nix` instead of using
`std::process::Process`.
