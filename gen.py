#!/usr/bin/env python3
"""
Wrapper script that proxies to Automation/generate_project_files.py
"""

import subprocess
import sys
from pathlib import Path


def main() -> int:
    script_dir = Path(__file__).resolve().parent
    generator = script_dir / "Automation" / "generate_project_files.py"

    if not generator.exists():
        print(f"Error: {generator} not found.")
        return 1

    cmd = [sys.executable, str(generator), *sys.argv[1:]]
    result = subprocess.run(cmd)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())


