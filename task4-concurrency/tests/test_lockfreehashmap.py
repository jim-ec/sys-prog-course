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

def sanity_check(output, n_buckets, initial, n_threads):
    count = 0
    for i in range(0, n_buckets):
        entries = output[i].split("-")
        count += len(entries) - 1
    if count > initial + n_threads:
        warn("Hashmap has more items than expected ")
        exit(1)

def main() -> None:
    # Run the test program
    test_lock_hashmap = test_root().joinpath("lockfree_hashmap")
    if not test_lock_hashmap.exists():
        run(["make", "-C", str(test_root()), str(test_lock_hashmap)])
    times = []
    with tempfile.TemporaryDirectory() as tmpdir:
        with subtest("Checking correctness"):
            with open(f"{tmpdir}/stdout", "w+") as stdout:
                run_project_executable(
                    str(test_lock_hashmap),
                    args=["-d20000", "-i10000", "-n4", "-r10000", "-u100", "-b1"],
                    stdout=stdout,
                )
                output = open(f"{tmpdir}/stdout").readlines()
                sanity_check(output[1:], 1, 10000, 4)
        with subtest("Checking 1 thread time"):
            with open(f"{tmpdir}/stdout", "w+") as stdout:
                runtime = 0.0
                for i in range(0, 3):
                    run_project_executable(
                        str(test_lock_hashmap),
                        args=["-d20000", "-i10000", "-n1", "-r10000", "-u10", "-b1"],
                        stdout=stdout,
                    )
                    output = open(f"{tmpdir}/stdout").readlines()
                    runtime += float(output[0].strip())
                    sanity_check(output[1:], 1, 10000, 1)
                times.append(runtime / 3)
        with subtest("Checking 2 thread time"):
            with open(f"{tmpdir}/stdout", "w+") as stdout:
                runtime = 0.0
                for i in range(0, 3):
                    run_project_executable(
                        str(test_lock_hashmap),
                        args=["-d20000", "-i10000", "-n2", "-r10000", "-u10", "-b1"],
                        stdout=stdout,
                    )
                    output = open(f"{tmpdir}/stdout").readlines()
                    runtime += float(output[0].strip())
                    sanity_check(output[1:], 1, 10000, 2)
                times.append(runtime / 3)
        with subtest("Checking 4 thread time"):
            with open(f"{tmpdir}/stdout", "w+") as stdout:
                runtime = 0.0
                for i in range(0, 3):
                    run_project_executable(
                        str(test_lock_hashmap),
                        args=["-d20000", "-i10000", "-n4", "-r10000", "-u10", "-b1"],
                        stdout=stdout,
                    )
                    output = open(f"{tmpdir}/stdout").readlines()
                    runtime += float(output[0].strip())
                    sanity_check(output[1:], 1, 10000, 4)
                times.append(runtime / 3)
    f1 = times[0] / times[1]
    f2 = times[1] / times[2]
    if f1 < 1.4 or f2 < 1.4:
        warn("Hashmap is not scaling properly: " + str(times))
        exit(1)


if __name__ == "__main__":
    main()
