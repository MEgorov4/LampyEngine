#!/usr/bin/env python3
"""
Generate Visual Studio project files for LampyEngine.
Wraps Conan + CMake generation similar to the LampyEngneRev test project.
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path


def run(cmd: list[str], cwd: Path) -> None:
    print("$", " ".join(str(part) for part in cmd))
    subprocess.run(cmd, check=True, cwd=cwd)


def copy_solution_with_fixed_paths(generated_dir: Path, project_root: Path, project_name: str) -> None:
    source = generated_dir / f"{project_name}.sln"
    dest = project_root / f"{project_name}.sln"

    if not source.exists():
        print(f"Warning: solution file not found at {source}")
        return

    sln_content = source.read_text(encoding="utf-8")
    # Ensure relative vcxproj paths point into build/generated
    prefix = "build\\generated"

    def repl(match: re.Match[str]) -> str:
        path = match.group(1)
        if path.startswith(prefix) or os.path.isabs(path):
            return match.group(0)
        if path.endswith(".vcxproj"):
            return f', "{prefix}\\{path}"'
        return match.group(0)

    updated = re.sub(r', "([^"]+\.vcxproj)"', repl, sln_content)
    dest.write_text(updated, encoding="utf-8")
    print(f"Copied solution to {dest}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate project files for LampyEngine",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""\
Examples:
  py gen.py -d      # Debug configuration
  py gen.py -r      # Release configuration
""",
    )

    group = parser.add_mutually_exclusive_group()
    group.add_argument("-d", "--debug", action="store_true", help="Generate Debug configuration (default)")
    group.add_argument("-r", "--release", action="store_true", help="Generate Release configuration")

    args = parser.parse_args()

    build_type = "Release" if args.release else "Debug"

    project_name = "LampyEngine"
    project_root = Path(__file__).resolve().parent.parent
    build_dir = project_root / "build"
    generated_dir = build_dir / "generated"
    third_party_dir = project_root / "Engine" / "ThirdParty"
    automation_dir = project_root / "Automation"

    conan_profile_name = f"conanProfile{build_type}"
    conan_profile = automation_dir / conan_profile_name
    if not conan_profile.exists():
        fallback = automation_dir / "conanProfileDebug"
        print(f"Warning: {conan_profile_name} not found, using {fallback.name}")
        conan_profile = fallback

    print()
    print(f"=== Generating {project_name} ({build_type}) ===")
    print()

    # Step 1: Conan install
    print("Step 1: Installing Conan dependencies...")
    conan_cmd = [
        "conan",
        "install",
        ".",
        "--output-folder",
        str(third_party_dir),
        "--build=missing",
        "--profile",
        str(conan_profile),
        "--profile:b",
        str(conan_profile),
    ]
    try:
        run(conan_cmd, project_root)
    except FileNotFoundError:
        print("Error: 'conan' command not found. Make sure Conan is installed and available in PATH.")
        return 1
    except subprocess.CalledProcessError as exc:
        print(f"Error: Conan install failed with exit code {exc.returncode}")
        return exc.returncode

    print("Conan dependencies installed successfully.\n")

    # Step 2: Ensure build directories
    print("Step 2: Creating build directories...")
    generated_dir.mkdir(parents=True, exist_ok=True)
    (build_dir / build_type).mkdir(parents=True, exist_ok=True)
    print(f"Created: {generated_dir}")
    print()

    # Step 3: Run CMake configure
    print("Step 3: Running CMake configure...")
    toolchain_file = third_party_dir / "conan_toolchain.cmake"
    cmake_cmd = [
        "cmake",
        "-S",
        str(project_root),
        "-B",
        str(generated_dir),
        "-G",
        "Visual Studio 17 2022",
        "-A",
        "x64",
        f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}",
        "--fresh",
    ]
    try:
        run(cmake_cmd, project_root)
    except FileNotFoundError:
        print("Error: 'cmake' command not found. Install CMake and add it to PATH.")
        return 1
    except subprocess.CalledProcessError as exc:
        print(f"Error: CMake configure failed with exit code {exc.returncode}")
        return exc.returncode

    print("CMake configuration completed successfully.\n")

    # Step 4: Copy solution to project root
    print("Step 4: Copying solution file...")
    copy_solution_with_fixed_paths(generated_dir, project_root, project_name)

    print()
    print(f"=== Done! Open {project_name}.sln ===")
    print()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())


