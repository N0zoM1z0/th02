#!/usr/bin/env python3
"""Fast TH02 edit/compile/link/compare loop in the ReC98 work tree.

    th02_quick.py main [--asm] [th02/foo.cpp ...]

Copies src/ and compat/rec98/ into the work tree (rec98_overlay.py), compiles
the given TUs (ReC98 paths) with the product's ReC98 flags, optionally reassembles
th02_<product>.asm, relinks from obj/th02/<product>.@l and compares the
program image with the pinned original. Differences are attributed to the
TLINK detailed-map module and nearest public symbol.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys

from lib.pc98 import parse_mz
import rec98_overlay
from lib.targets import find_artifact, load_target_manifest, read_verified_artifact

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / ".analysis" / "work" / "rec98"
BINARY = {"op": "O", "main": "M", "maine": "E"}
LINE = re.compile(r"^\s*([0-9A-F]{4}):([0-9A-F]{4})\s+([0-9A-F]{4,5})\s+C=(\S+)\s+S=(\S+)\s+G=(\S*)\s+M=(.+?)\s+ACBP")
PUB = re.compile(r"^\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(?:idle\s+|Abs\s+)?(\S.*?)\s*$")


def environment() -> dict[str, str]:
    env = os.environ.copy()
    env.update({"DISPLAY": "", "WAYLAND_DISPLAY": "", "WINEDEBUG": "-all", "MSDOS_PATH": r"C:\TC4\BIN",
                "PATH": f"{ROOT / '.tools' / 'factory-bin'}{os.pathsep}{env.get('PATH', '')}"})
    return env


def run(command: list[str], work: Path) -> str:
    result = subprocess.run(["nice", "-n", "10", *command], cwd=work, env=environment(),
                            capture_output=True, text=True, errors="replace")
    output = "\n".join(line for line in (result.stdout + result.stderr).splitlines()
                       if line.strip() and not line.startswith("wine:"))
    if result.returncode or re.search(r"\bError\b|\*\*Error\*\*|Fatal", output):
        print(output)
        raise SystemExit(f"failed: {' '.join(command[:3])}")
    return output


def symbols(map_text: str) -> tuple[list[tuple[int, int, str]], list[tuple[int, str]]]:
    detail = map_text.split("Detailed map of segments", 1)[1].split("Address", 1)[0]
    modules = []
    for line in detail.splitlines():
        m = LINE.match(line)
        if m:
            start = int(m[1], 16) * 16 + int(m[2], 16)
            modules.append((start, start + int(m[3], 16), f"{m[7]}:{m[5]}"))
    publics = []
    for line in map_text.split("Publics by Value", 1)[1].splitlines():
        m = PUB.match(line)
        if m:
            publics.append((int(m[1], 16) * 16 + int(m[2], 16), m[3]))
    publics.sort()
    return modules, publics


def where(offset: int, modules, publics) -> str:
    module = next((name for start, end, name in modules if start <= offset < end), "gap")
    near = [p for p in publics if p[0] <= offset]
    sym = f"{near[-1][1]}+{offset - near[-1][0]:#x}" if near else "?"
    return f"{module} {sym}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("product", choices=sorted(BINARY))
    parser.add_argument("sources", nargs="*")
    parser.add_argument("--asm", action="store_true", help=f"reassemble th02_<product>.asm")
    parser.add_argument("--work", type=Path, default=WORK)
    parser.add_argument("--show", type=int, default=12)
    args = parser.parse_args()
    work, product = args.work, args.product
    rec98_overlay.apply(work)

    if args.sources:
        response = work / "obj" / "quick.@c"
        response.write_text(
            f"-c -I. -O -b- -3 -Z -d -DGAME=2 -ml -DBINARY='{BINARY[product]}' -nobj/th02/ "
            + " ".join(args.sources) + "\r\n", encoding="ascii")
        out = run(["wine", "bin/msdos.exe", "-e", "-x", "tcc", r"@obj\quick.@c"], work)
        warnings = [l for l in out.splitlines() if "Warning" in l]
        if warnings:
            print("\n".join(warnings))
    if args.asm:
        run(["wine", r"C:\TASM50\bin\TASM32.EXE", "/m", "/mx", "/kh32768", "/t", "/dGAME=2",
             f"th02_{product}.asm", rf"obj\th02\{product}.obj"], work)
    run(["wine", "bin/msdos.exe", "-e", "-x", "tlink", rf"@obj\th02\{product}.@l"], work)

    manifest = load_target_manifest(ROOT / "config" / "targets.toml")
    target = parse_mz(read_verified_artifact(ROOT, find_artifact(manifest, f"th02-{product}"))).program_image
    image = parse_mz((work / "bin" / "th02" / f"{product}.exe").read_bytes())
    built = image.program_image
    modules, publics = symbols((work / "obj" / "th02" / f"{product}.map").read_text(encoding="latin-1"))
    diffs = [i for i in range(min(len(target), len(built))) if target[i] != built[i]]
    target_mz = parse_mz(read_verified_artifact(ROOT, find_artifact(manifest, f"th02-{product}")))
    reloc = lambda mz: sorted((r.segment, r.offset) for r in mz.relocations)
    relocs_equal = reloc(target_mz) == reloc(image)
    print(f"{product}: size {len(built)} (target {len(target)}), differing bytes {len(diffs)}, "
          f"relocation set {'equal' if relocs_equal else 'DIFFERS'}")
    shown = 0
    last = -100
    for i in diffs:
        if i - last > 8 and shown < args.show:
            print(f"  {i:05X}  T={target[i:i+8].hex(' ')}  B={built[i:i+8].hex(' ')}  {where(i, modules, publics)}")
            shown += 1
        last = i
    return 0 if not diffs and len(target) == len(built) and relocs_equal else 1


if __name__ == "__main__":
    sys.exit(main())
