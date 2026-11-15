#!/usr/bin/env python3
"""Install rebuilt TH02 programs into a disposable copy of the original game image.

The source is the hash-attested Japanese HDI retained by
`import_targets.py --retain-runtime-image`. Its four programs must still match
config/targets.toml; they are replaced with the outputs of a
`build_th02.py` run, everything else (game data, KAJA's drivers, DOS) stays
original. `--original` keeps the original programs as a baseline.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import tomllib

from lib.pc98 import digest_file

ROOT = Path(__file__).resolve().parents[1]
PROGRAMS = {"th02-op": "OP.EXE", "th02-main": "MAIN.EXE", "th02-maine": "MAINE.EXE", "th02-zun": "ZUN.COM"}


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def mtools(*args: str) -> None:
    env = dict(os.environ, MTOOLS_SKIP_CHECK="1")
    subprocess.run(args, check=True, capture_output=True, env=env)


def read(image: str, dos_path: str, scratch: Path) -> bytes:
    out = scratch / Path(dos_path).name
    out.unlink(missing_ok=True)
    mtools("mcopy", "-n", "-i", image, f"::{dos_path}", str(out))
    return out.read_bytes()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--build-run", type=Path, help="a build_th02.py run directory")
    group.add_argument("--original", action="store_true", help="keep the original programs")
    parser.add_argument("--output-dir", type=Path, required=True, help="new directory below .analysis/runtime")
    args = parser.parse_args()

    output = args.output_dir.resolve()
    if output.exists() or not output.is_relative_to(ROOT / ".analysis" / "runtime"):
        parser.error("use a new directory below .analysis/runtime")
    imported = json.loads((ROOT / ".analysis" / "target-import.json").read_text(encoding="utf-8"))["hdi"]
    source = ROOT / imported["private_runtime_path"]
    if digest_file(source) != imported["sha256"]:
        sys.exit(f"{source}: differs from the imported image")
    targets = {a["id"]: a for a in tomllib.loads((ROOT / "config" / "targets.toml").read_text())["artifacts"]}

    output.mkdir(parents=True)
    hdi = output / "game.hdi"
    shutil.copyfile(source, hdi)
    image = f"{hdi}@@{imported['fat']['offset']}"
    installed = {}
    with tempfile.TemporaryDirectory(dir=output) as raw:
        scratch = Path(raw)
        for artifact_id, name in PROGRAMS.items():
            target = targets[artifact_id]
            dos_path = target["dos_path"]
            if sha256(read(image, dos_path, scratch)) != target["sha256"]:
                sys.exit(f"{dos_path} in the image is not the pinned original")
            if args.original:
                continue
            built = args.build_run.resolve() / "source" / "bin" / "th02" / name.lower()
            data = built.read_bytes()
            mtools("mcopy", "-o", "-i", image, str(built), f"::{dos_path}")
            if read(image, dos_path, scratch) != data:
                sys.exit(f"{dos_path}: readback differs from {built}")
            installed[name] = {"source": str(built), "size": len(data), "sha256": sha256(data),
                               "equals_original": sha256(data) == target["sha256"]}
    receipt = {
        "programs": "original" if args.original else installed,
        "build_run": None if args.original else str(args.build_run.resolve()),
        "source_hdi_sha256": imported["sha256"],
        "game_hdi_sha256": digest_file(hdi),
    }
    (output / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    print(hdi)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
