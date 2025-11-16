#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


def add_data_argument(source: Path, destination: str) -> list[str]:
    if not source.exists():
        return []
    return ["--add-data", f"{source}{os.pathsep}{destination}"]


def ensure_clean_directory(directory: Path) -> None:
    if directory.exists():
        shutil.rmtree(directory, ignore_errors=True)
    directory.mkdir(parents=True, exist_ok=True)


def main() -> None:
    parser = argparse.ArgumentParser(description="Build ProjectBrowser executable with PyInstaller.")
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--dist", required=True, type=Path)
    parser.add_argument("--build", required=True, type=Path)
    parser.add_argument("--spec", required=True, type=Path)
    parser.add_argument("--name", required=True)
    parser.add_argument("--python", default=sys.executable, help="Python interpreter to use.")
    args = parser.parse_args()

    root = args.root.resolve()
    dist_dir = args.dist.resolve()
    build_dir = args.build.resolve()
    spec_dir = args.spec.resolve()
    main_py = root / "main.py"

    if not main_py.exists():
        raise FileNotFoundError(f"ProjectBrowser main.py not found: {main_py}")

    ensure_clean_directory(dist_dir)
    ensure_clean_directory(build_dir)
    spec_dir.mkdir(parents=True, exist_ok=True)

    command = [
        args.python,
        "-m",
        "PyInstaller",
        str(main_py),
        "--name",
        args.name,
        "--clean",
        "--noconfirm",
        "--noconsole",
        "--onefile",
        "--distpath",
        str(dist_dir),
        "--workpath",
        str(build_dir),
        "--specpath",
        str(spec_dir),
    ]

    for directory in ("src", "Templates", "SampleProject"):
        command += add_data_argument(root / directory, directory)

    for asset in ("recent.json", "app_icon.png", "project-browser-dark.png"):
        asset_path = root / asset
        if asset_path.exists():
            command += add_data_argument(asset_path, ".")

    env = os.environ.copy()
    env.setdefault("PYTHONUTF8", "1")
    print("Executing:", " ".join(command))
    subprocess.check_call(command, cwd=root, env=env)


if __name__ == "__main__":
    main()



