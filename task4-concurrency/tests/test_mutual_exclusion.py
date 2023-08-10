#!/usr/bin/env python3

import os
import tempfile

from testsupport import (
    run,
    subtest,
    warn,
    test_root,
    run_project_executable,
    ensure_library,
)


def main() -> None:
    # Run the test program
    test_mutual_exclusion = test_root().joinpath("test_mutual_exclusion")
    if not test_mutual_exclusion.exists():
        run(["make", "-C", str(test_root()), str(test_mutual_exclusion)])

    with subtest("Checking mutual exclusion"):
        run_project_executable(str(test_mutual_exclusion))


if __name__ == "__main__":
    main()
