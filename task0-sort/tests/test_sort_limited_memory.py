#!/usr/bin/env python3

import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from shlex import quote
from typing import Any, IO

from testsupport import (
    subtest,
    run,
    warn,
    project_path,
    find_executable,
)
from sorttest_helpers import download_wiki, ensure_dependencies


def run_with_ulimit(exe: str, stdin: IO[Any], stdout: IO[Any]) -> None:
    # size is in kilobytes
    size = 128 * 1024
    run(
        [f"ulimit -v {quote(str(size))}; {quote(str(exe))}"],
        stdin=stdin,
        stdout=stdout,
        extra_env=dict(LC_ALL="C"),
        shell=True,
    )


def main() -> None:
    path = download_wiki("en-latest-all-titles-in")

    ensure_dependencies()

    own_sort_exe = find_executable("sort", project_path())
    if own_sort_exe is None:
        warn(f"executable 'sort' not found in {project_path()}")
        sys.exit(1)

    with TemporaryDirectory() as tempdir:
        temp_path = Path(tempdir)
        coreutils_sort = temp_path.joinpath(path.name + ".coreutils-sort")
        own_sort = temp_path.joinpath(path.name + ".own-sort")

        with subtest("Run coreutils sort with 128MB limit"):
            with open(path) as stdin, open(coreutils_sort, "w") as stdout:
                run_with_ulimit("sort", stdin, stdout)

        with subtest("Run own sort with 128MB limit"):
            with open(path) as stdin, open(own_sort, "w") as stdout:
                run_with_ulimit(own_sort_exe, stdin, stdout)

        with subtest("Check if both results matches"):
            run(["cmp", str(coreutils_sort), str(own_sort)])


if __name__ == "__main__":
    main()
