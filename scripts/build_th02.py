#!/usr/bin/env python3
"""Build TH02 from pinned ReC98 plus this repository's patches, then compare.

1. `git archive` the pinned ReC98 revision into a fresh private run directory.
2. Apply every `patches/rec98/*.patch` in name order (`patch -p1`).
3. Run ReC98's `build.bat` with the attested Borland toolchain under Wine.
4. Write the per-contribution ledger and print the strict raw comparison.

Without `--write-ledger` the ledger goes into the run directory only.
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import re
import subprocess
import sys

from cold_build_rec98 import REFERENCE, REVISION
from lib.pc98 import digest_file


ROOT = Path(__file__).resolve().parents[1]
PATCHES = ROOT / "patches" / "rec98"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run-id", default=datetime.now(timezone.utc).strftime("th02-%Y%m%dT%H%M%SZ"))
    parser.add_argument("--no-patches", action="store_true", help="build unpatched ReC98")
    parser.add_argument("--write-ledger", action="store_true", help="update config/rec98_th02_units.csv")
    args = parser.parse_args()
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_.-]*", args.run_id):
        parser.error("--run-id must contain only letters, digits, dot, underscore, or dash")

    run_root = ROOT / ".analysis" / "builds" / f"th02-{REVISION[:10]}" / args.run_id
    if run_root.exists():
        print(f"refusing to overwrite prior run: {run_root}", file=sys.stderr)
        return 1
    source = run_root / "source"
    source.mkdir(parents=True)
    archive = subprocess.run(["git", "archive", "--format=tar", REVISION], cwd=REFERENCE,
                             check=True, capture_output=True).stdout
    subprocess.run(["tar", "-x", "-C", str(source)], input=archive, check=True)

    patches = [] if args.no_patches else sorted(PATCHES.glob("*.patch"))
    for patch in patches:
        subprocess.run(["patch", "-p1", "--forward", "--batch", "--quiet", "-i", str(patch)],
                       cwd=source, check=True)

    environment = os.environ.copy()
    environment.update({
        "DISPLAY": "", "WAYLAND_DISPLAY": "", "WINEDEBUG": "-all", "MSDOS_PATH": r"C:\TC4\BIN",
        "PATH": f"{ROOT / '.tools' / 'factory-bin'}{os.pathsep}{environment.get('PATH', '')}",
    })
    command = ["wine", "cmd", "/d", "/c",
               r"set PATH=C:\TASM50\BIN;C:\TC4\BIN;%PATH%&&set PROCESSOR_ARCHITECTURE=AMD64"
               r"&&set PROCESSOR_ARCHITEW6432=AMD64&&build.bat"]
    with (run_root / "build.log").open("wb") as log:
        completed = subprocess.run(["nice", "-n", "10", *command], cwd=source, env=environment,
                                   stdout=log, stderr=subprocess.STDOUT)
    outputs = {path.name: digest_file(path) for path in sorted((source / "bin" / "th02").glob("*"))}
    receipt = {
        "rec98_revision": REVISION,
        "patches": {path.name: digest_file(path) for path in patches},
        "returncode": completed.returncode,
        "th02_outputs": outputs,
    }
    (run_root / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    if completed.returncode:
        print(f"build FAILED; see {run_root / 'build.log'}", file=sys.stderr)
        return completed.returncode

    ledger = ROOT / "config" / "rec98_th02_units.csv" if args.write_ledger else run_root / "rec98_th02_units.csv"
    subprocess.run([sys.executable, str(ROOT / "scripts" / "rec98_th02_ledger.py"), str(source),
                    "--output", str(ledger)], check=True)
    print(f"run: {run_root}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
