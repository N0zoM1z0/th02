#!/usr/bin/env python3
"""Copy this repository's maintained files into a pinned ReC98 tree.

config/rec98_overlay.toml maps each file under src/ and compat/rec98/ to its
path in ReC98. Files are copied byte-for-byte. With no argument, the target is
the private work tree .analysis/work/rec98 used by scripts/th02_quick.py.
"""

from __future__ import annotations

import argparse
import shutil
import tomllib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "config" / "rec98_overlay.toml"


def files() -> dict[str, str]:
    return tomllib.loads(MANIFEST.read_text(encoding="utf-8"))["files"]


def apply(tree: Path) -> list[str]:
    changed = []
    for local, upstream in files().items():
        source = ROOT / local
        target = tree / upstream
        if target.is_file() and target.read_bytes() == source.read_bytes():
            continue
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, target)
        changed.append(upstream)
    return changed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("tree", nargs="?", type=Path, default=ROOT / ".analysis" / "work" / "rec98")
    args = parser.parse_args()
    for path in apply(args.tree):
        print(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
