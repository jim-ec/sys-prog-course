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

        with subtest("Create hierarchical directories with distinct names"):
            dir_list = [
                    "foo", "bar", "foobar", "barfoo", 
                    os.path.join("foo", "b"),
                    os.path.join("foo", "f"),
                    os.path.join("", *["foo", "b", "foob"]),
                    os.path.join("", *["foo", "b", "ba"]),
                ]
            for dir in dir_list:
                os.mkdir(os.path.join(mnt_path, dir))

            flat_dir_list = ["foo", "bar", "foobar", "barfoo"]
            check_dir_list = os.listdir(mnt_path)
            for dir in flat_dir_list:
                if dir not in check_dir_list:
                    fuse_unmount(mnt_path)
                    sys.exit(1)
            
            hier_dir_level_one = ["f", "b"]
            check_dir_list = os.listdir(os.path.join(mnt_path, "foo"))
            for dir in hier_dir_level_one:
                if dir not in check_dir_list:
                    fuse_unmount(mnt_path)
                    sys.exit(1)

            hier_dir_level_two = ["foob", "ba"]
            check_dir_list = os.listdir(os.path.join(mnt_path, os.path.join("foo", "b")))
            for dir in hier_dir_level_two:
                if dir not in check_dir_list:
                    fuse_unmount(mnt_path)
                    sys.exit(1)

        fuse_unmount(mnt_path)
        sys.exit(0)

if __name__ == "__main__":
    main()