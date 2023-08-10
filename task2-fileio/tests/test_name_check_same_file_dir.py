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

        with subtest("Create files and dir with same name in different dirs"):
            node_list = [
                    "foo",
                    os.path.join("foo", "foo"),
                    os.path.join("", *["foo", "foo", "foo"])
            ]
            
            try:
                os.mkdir(os.path.join(mnt_path, node_list[0]))
            except Exception as e:
                print(e)
                fuse_unmount(mnt_path)
                sys.exit(1)
            
            try:
                os.mkdir(os.path.join(mnt_path, node_list[1]))
            except Exception as e:
                print(e)
                fuse_unmount(mnt_path)
                sys.exit(1)
            
            try:
                os.mknod(os.path.join(mnt_path, node_list[2]))
            except Exception as e:
                print(e)
                fuse_unmount(mnt_path)
                sys.exit(1)

            if os.listdir(mnt_path) != ["foo"]:
                fuse_unmount(mnt_path)
                sys.exit(1)
                        
            if os.listdir(os.path.join(mnt_path, node_list[1])) != ["foo"]:
                fuse_unmount(mnt_path)
                sys.exit(1)
            
        fuse_unmount(mnt_path)
        sys.exit(0)

if __name__ == "__main__":
    main()