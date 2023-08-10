#!/usr/bin/env python3

import gzip
import os
import shutil
from pathlib import Path

from testsupport import ensure_download, test_root, assert_executable

URL_PREFIX = "https://github.com/Mic92/wiki-topics/releases/download/assets/"


def ensure_dependencies() -> None:
    assert_executable("sort", "This test requires 'sort' command line tool")
    assert_executable("cmp", "This test requires 'cmp' command line tool")


def download_wiki(name: str) -> Path:
    compressed = test_root().joinpath(name + ".gz")
    uncompressed = test_root().joinpath(name)
    uncompressed_temp = test_root().joinpath(name + ".tmp")
    ensure_download(f"{URL_PREFIX}/{name}.gz", compressed)
    if uncompressed.exists():
        return uncompressed

    with gzip.open(compressed, "rb") as f_in, open(uncompressed_temp, "wb") as f_out:
        shutil.copyfileobj(f_in, f_out)
        os.rename(uncompressed_temp, uncompressed)
    return uncompressed
