#!/usr/bin/env python3
"""Check TH02 targets and toolchain before target-dependent work."""

from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    for script, arguments in [("check_environment.py", []), ("attest_toolchain.py", []),
                              ("verify_targets.py", ["--game", "th02"])]:
        print(f"[{script}]", flush=True)
        result = subprocess.run([sys.executable, f"scripts/{script}", *arguments], cwd=ROOT)
        if result.returncode:
            return result.returncode
    print("preflight: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
