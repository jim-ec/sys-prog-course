#!/usr/bin/env python3

import os
import subprocess
import resource
import tempfile
import time
from shlex import quote

from testsupport import (
    info,
    run_project_executable,
    subtest,
    warn,
    find_project_executable,
)
from processtest_helpers import ensure_dependencies


test_line = "grep-should-find-this"


def main() -> None:
    ensure_dependencies()

    with subtest("Test shell builtin exit"):
        proc = run_project_executable(
            "shell",
            input=f"exit -1\nsleep 60",
            stdout=subprocess.PIPE,
            timeout=2,
            check=False,
        )
        assert (
            proc.returncode == 255
        ), f"expected exit to terminate with exit code 255, got: {proc.returncode}"
        info("OK")

        proc = run_project_executable(
            "shell",
            input=f"exit 1\nsleep 60",
            stdout=subprocess.PIPE,
            timeout=2,
            check=False,
        )
        assert (
            proc.returncode == 1
        ), f"expected exit to terminate with exit code 1, got: {proc.returncode}"
        info("OK")

        proc = run_project_executable(
            "shell",
            input=f"exit\nsleep 60",
            stdout=subprocess.PIPE,
            timeout=2,
            check=False,
        )
        assert (
            proc.returncode == 0
        ), f"expected exit to terminate with exit code 1, got: {proc.returncode}"
        info("OK")

    with tempfile.TemporaryDirectory() as dir:
        path = f"{dir}/file"
        cmd = f'bash -c "sleep 1; echo subshell >> {path}" &\necho shell > {path}\n'
        with subtest(f"Test background execution by running {cmd}"):
            proc = run_project_executable(
                "shell", input=cmd, stdout=subprocess.PIPE, timeout=3
            )
            assert os.path.isfile(path), f"command did not create a file at {path}"
            with open(path) as f:
                content = f.read()
            expected = "shell\nsubshell\n"
            assert (
                content == expected
            ), f"{path} does not contain expected content: '{expected}', got:\n'{content}'"
            info("OK")

    cmd = f'bash -c "sleep 1; echo subshell" &\nwait\necho shell\n'

    with subtest(f"Test shell builtin wait with {cmd}"):
        proc = run_project_executable("shell", input=cmd, stdout=subprocess.PIPE)
        expected = "subshell\nshell\n"
        assert (
            proc.stdout == expected
        ), f"expected shell output: '{expected}', got:\n'{proc.stdout}'"
        info("OK")

    with subtest(f"Test shell builtin kill"):
        with subprocess.Popen(["cat"], stdin=subprocess.PIPE, text=True) as cat_proc:
            cmd = f"kill {cat_proc.pid}\n"
            proc = run_project_executable("shell", input=cmd, stdout=subprocess.PIPE)
            signal = cat_proc.poll()
            expected_signal = -15
            assert (
                signal == expected_signal
            ), f"expected kill to terminate process with SIGTERM (-15), got: {signal}"
            info("OK")


if __name__ == "__main__":
    main()
