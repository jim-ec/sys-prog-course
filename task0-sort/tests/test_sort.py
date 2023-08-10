#!/usr/bin/env python3

from pathlib import Path
from tempfile import TemporaryDirectory

from testsupport import run, run_project_executable, subtest
from sorttest_helpers import download_wiki, ensure_dependencies


def main() -> None:
    path = download_wiki("scowiki-latest-all-titles-in")

    ensure_dependencies()

    with TemporaryDirectory() as tempdir:
        temp_path = Path(tempdir)

        coreutils_sort = temp_path.joinpath(path.name + ".coreutils-sort")
        with subtest("Run coreutils sort"):
            with open(path) as stdin, open(coreutils_sort, "w") as stdout:
                run(["sort"], stdin=stdin, stdout=stdout, extra_env=dict(LC_ALL="C"))

        own_sort = temp_path.joinpath(path.name + ".own-sort")
        with subtest("Run own sort"):
            with open(path) as stdin, open(own_sort, "w") as stdout:
                run_project_executable("sort", stdin=stdin, stdout=stdout)

        with subtest("Check if both results matches"):
            run(["cmp", str(coreutils_sort), str(own_sort)])


if __name__ == "__main__":
    main()
