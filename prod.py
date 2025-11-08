#!/usr/bin/env python3
"""
Build and install the LampyEngine SDK.
Usage examples:
  py prod.py                    # Release build/install, default prefix <repo>/SDK-Install
  py prod.py --prefix C:/SDK    # Custom install prefix
  py prod.py --config Debug     # Debug configuration
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def run(cmd: list[str], cwd: Path) -> None:
    print("$", " ".join(str(part) for part in cmd))
    subprocess.run(cmd, check=True, cwd=cwd)


def main() -> int:
    parser = argparse.ArgumentParser(description="Build and install the LampyEngine SDK")
    parser.add_argument("--prefix", help="Install prefix (default: <repo>/SDK-Install)")
    parser.add_argument(
        "--config",
        default="Release",
        choices=["Release", "Debug", "RelWithDebInfo", "MinSizeRel"],
        help="CMake configuration to build/install",
    )
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent
    generated_dir = project_root / "build" / "generated"
    install_prefix = Path(args.prefix) if args.prefix else project_root / "SDK-Install"

    # 1) Generate project files for requested config
    gen_flag = "-r" if args.config == "Release" else "-d"
    run([sys.executable, str(project_root / "gen.py"), gen_flag], cwd=project_root)

    # 2) Build
    run(["cmake", "--build", str(generated_dir), "--config", args.config], cwd=project_root)

    # 3) Install SDK
    run(
        ["cmake", "--install", str(generated_dir), "--config", args.config, "--prefix", str(install_prefix)],
        cwd=project_root,
    )

    print()
    print(f"SDK installed to: {install_prefix}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        print(f"Error: command failed with exit code {exc.returncode}")
        raise SystemExit(exc.returncode)


