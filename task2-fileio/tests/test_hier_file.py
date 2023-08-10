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

        with subtest("Create hierarchical files"):
            dir_list = [
                    "foo", 
                    os.path.join("foo", "bar"),
            ]
            for dir in dir_list:
                os.mkdir(os.path.join(mnt_path, dir))

            file_list = [
                "hello",
                os.path.join("foo", "hello_one"),
                os.path.join("", *["foo", "bar", "hello_two"])
            ]
            for file in file_list:
                os.mknod(os.path.join(mnt_path, file))
            
            flat_file_list = ["hello"]
            check_dir_list = os.listdir(mnt_path)
            for file in flat_file_list:
                if file not in check_dir_list:
                    fuse_unmount(mnt_path)
                    sys.exit(1)
                        
            hier_file_list_level_one = ["hello_one"]
            check_dir_list = os.listdir(os.path.join("", *[mnt_path, "foo"]))
            for file in hier_file_list_level_one:
                if file not in check_dir_list:
                    fuse_unmount(mnt_path)
                    sys.exit(1)
            
            hier_file_list_level_two = ["hello_two"]
            check_dir_list = os.listdir(os.path.join("", *[mnt_path, "foo", "bar"]))
            for file in hier_file_list_level_two:
                if file not in check_dir_list:
                    fuse_unmount(mnt_path)
                    sys.exit(1)
            
        fuse_unmount(mnt_path)
        sys.exit(0)

if __name__ == "__main__":
    main()