#!/usr/bin/env python3

import os
import socket
import subprocess
import tempfile

from testsupport import info, run_project_executable, warn, subtest
from processtest_helpers import ensure_dependencies


def main() -> None:
    ensure_dependencies()
    hostname = socket.gethostname()

    with subtest("Test running simple command 'echo hostname'"):
        proc = run_project_executable(
            "shell", input=f"echo {hostname}\n", stdout=subprocess.PIPE
        )
        out = proc.stdout.strip()
        assert out == hostname, f"shell stdout: expected: {hostname}, got {out}"
        info("OK")

    with subtest("Test number of file descriptor with 'ls /proc/self/fd'"):
        proc = run_project_executable(
            "shell", input="ls /proc/self/fd\n", stdout=subprocess.PIPE
        )
        out = proc.stdout
        expected = "0\n1\n2\n3\n"
        assert (
            out == expected
        ), f"Child process has not expected number of file descriptors: expected: {expected}, got {out}"
        info("OK")

    expected_stderr = os.readlink("/proc/self/fd/2")
    with subtest(
        "Test open file descriptor 'readlink /proc/self/fd/0 /proc/self/fd/1 /proc/self/fd/2 /proc/self/fd/3'"
    ):
        proc = run_project_executable(
            "shell",
            input="readlink /proc/self/fd/0 /proc/self/fd/1 /proc/self/fd/2 /proc/self/fd/3\n",
            stdout=subprocess.PIPE,
        )
        out = proc.stdout
        lines = out.strip().split("\n")
        assert len(lines) == 3, f"Expected 3 lines in the readlink output, got:\n{out}"
        stdin = lines[0]
        assert stdin.startswith(
            "pipe:"
        ), f"Expected stdin (fd=0) to be a pipe, got: '{stdin}'"
        stdout = lines[1]
        assert stdout.startswith(
            "pipe:"
        ), f"Expected stdout (fd=1) to be a pipe, got: '{stdout}'"
        stderr = lines[2]
        assert (
            stderr == expected_stderr
        ), f"Expected stderr (fd=2): '{expected_stderr}', got: '{stderr}'"
        info("OK")

    with subtest("Test shell rediretion 'readlink /proc/self/fd/0 < /dev/null'"):
        proc = run_project_executable(
            "shell",
            input="readlink /proc/self/fd/0 < /dev/null\n",
            stdout=subprocess.PIPE,
        )
        out = proc.stdout.strip()
        expected = "/dev/null"
        assert (
            out == expected
        ), f"Child process has not expected number of file descriptors: expected: '{expected}', got '{out}'"
        info("OK")

    with tempfile.TemporaryDirectory() as dir:
        dest = f"{dir}/file"
        with subtest(
            f"Test shell redirection with 'readlink /proc/self/fd/1 > {dir}/file'"
        ):
            proc = run_project_executable(
                "shell",
                input=f"readlink /proc/self/fd/1 > {dest}\n",
                stdout=subprocess.PIPE,
            )
            out = proc.stdout.strip()
            assert (
                out == ""
            ), f"Child process output from stdout should be empty but got '{out}'"
            assert os.path.isfile(
                dest
            ), f"Expected shell redirection to create {dest}, however file does not exists"
            try:
                with open(dest) as f:
                    content = f.read().strip()
            except IOError as e:
                warn(
                    f"Expected shell redirection to create a file with readable permission but got Exception {e} when opening it"
                )
                exit(1)
            assert (
                content == dest
            ), f"Expected {dest} created by shell redirection to contain '{dest}' but got: '{content}'"
            info("OK")


if __name__ == "__main__":
    main()
