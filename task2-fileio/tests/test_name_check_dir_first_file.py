#!/usr/bin/env python3

import sys, os, tempfile
from pathlib import Path

from testsupport import run, run_project_executable, run_find_project_executable, subtest
from fuse_helpers import run_background, fuse_unmount, fuse_mount, gen_mnt_path, fuse_check_mnt

def main() -> None:
    with tempfile.TemporaryDirectory() as tmpdir:
        temp_path = Path(tmpdir)
        mnt_path  = fuse_mount(temp_path, "memfs_mnt")

        fuse_check_mnt(tmpdir, mnt_path)

        with subtest("Check that file and dir cannot have same name in same dir when file is created first"):
            file_list = ["foo"]
            for file in file_list:
                os.mkdir(os.path.join(mnt_path, file))

            try:
                os.mknod(os.path.join(mnt_path, file_list[0]))
                fuse_unmount(mnt_path)
                sys.exit(1)
            except FileExistsError as e:
                fuse_unmount(mnt_path)
                sys.exit(0)

if __name__ == "__main__":
    main()