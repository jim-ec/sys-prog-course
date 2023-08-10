#!/usr/bin/env python3

import sys, os, tempfile, subprocess
from pathlib import Path

from testsupport import run, run_project_executable, run_find_project_executable, subtest
from fuse_helpers import run_background, fuse_unmount, fuse_mount, gen_mnt_path, fuse_check_mnt

def main() -> None:
    with tempfile.TemporaryDirectory() as tmpdir:
        temp_path = Path(tmpdir)
        mnt_path  = fuse_mount(temp_path, "memfs_mnt")

        fuse_check_mnt(tmpdir, mnt_path)

        with subtest(""):
            run([os.path.join("tests", "file_io.sh"), "-o", os.path.join(mnt_path, "foo"), "hello"])
            proc = run([os.path.join("tests", "file_io.sh"), "-d", os.path.join(mnt_path, "foo")], stdout=subprocess.PIPE)

            try:
                if proc.stdout != "hello\n":
                    fuse_unmount(mnt_path)
                    print("Written and read data don't match")
                    sys.exit(1)
            except Exception as e:
                fuse_unmount(mnt_path)
                sys.exit(1)
        fuse_unmount(mnt_path)
        sys.exit(0)

if __name__ == "__main__":
    main()