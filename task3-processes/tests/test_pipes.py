#!/usr/bin/env python3

import subprocess
import os
import resource

from testsupport import info, run_project_executable, subtest
from processtest_helpers import ensure_dependencies


test_line = "grep-should-find-this"


def main() -> None:
    ensure_dependencies()

    src_file = os.path.abspath(__file__)

    with subtest(
        f"Test pipe implementation by running 'cat {src_file} | grep {test_line}'"
    ):
        proc = run_project_executable(
            "shell",
            input=f"cat {src_file} | grep {test_line}\n",
            stdout=subprocess.PIPE,
        )
        out = proc.stdout.strip()
        expected = f'test_line = "{test_line}"'
        assert out == expected, f"expect pipe output to be: {expected}, got '{out}'"
        info("OK")

    with subtest(
        "Test that pipe does not block for large input with 'dd if=/dev/zero count=4096 bs=4096 | wc -c'"
    ):
        proc = run_project_executable(
            "shell",
            input="dd if=/dev/zero count=4096 bs=4096 | wc -c\n",
            stdout=subprocess.PIPE,
        )
        out = proc.stdout.strip()
        expected = f"{4096 * 4096}"
        assert out == expected, f"expect pipe output to be: '{expected}', got '{out}'"
        info("OK")

    with subtest("Test if shell leaks file descriptors"):
        resource.setrlimit(resource.RLIMIT_NOFILE, (10, 10))
        cmds = "".join(f"echo line{n + 1} | cat\n" for n in range(100))
        proc = run_project_executable(
            "shell",
            input=cmds,
            stdout=subprocess.PIPE,
        )
        out = proc.stdout.strip()
        lines = out.split("\n")
        assert (
            len(lines) == 100
        ), f"expect 100 lines in pipe output, got {len(lines)} lines:\n{out}"
        info("OK")


if __name__ == "__main__":
    main()
