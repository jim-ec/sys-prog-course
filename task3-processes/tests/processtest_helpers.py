#!/usr/bin/env python3

from testsupport import assert_executable


def ensure_dependencies() -> None:
    cmds = ["sleep", "echo", "readlink", "grep", "cat", "wc", "dd", "bash"]
    for cmd in cmds:
        assert_executable(cmd, f"This test requires '{cmd}' command line tool")
