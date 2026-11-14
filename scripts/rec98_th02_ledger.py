#!/usr/bin/env python3
"""Build the TH02 ReC98 import ledger from one cold build.

Reads each product's TLINK detailed segment map, attributes every module
contribution to a ReC98 source, a ReC98 assembly file or the Borland runtime
library, and compares the contribution's bytes with the pinned Japanese
target. Output is config/rec98_th02_units.csv (one row per contribution).

Only OP, MAIN and MAINE are mapped EXEs. ZUN.COM is checked as a whole file.
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import re
import sys

from lib.pc98 import digest_file, parse_mz
from lib.targets import find_artifact, load_target_manifest, read_verified_artifact


ROOT = Path(__file__).resolve().parents[1]
LEDGER = ROOT / "config" / "rec98_th02_units.csv"
PRODUCTS = ("op", "main", "maine")
INITIALIZED = {"CODE", "DATA", "CONST", "INITDATA", "EXITDATA", "FAR_DATA"}
LINE = re.compile(
    r"^\s*([0-9A-F]{4}):([0-9A-F]{4})\s+([0-9A-F]{4,5})\s+C=(\S+)\s+S=(\S+)\s+G=(\S*)\s+M=(.+?)\s+ACBP=([0-9A-F]+)\s*$"
)


def kind_of(module: str) -> str:
    name = module.replace("\\", "/")
    if name.endswith((".c", ".cpp")):
        return "rec98-c"
    if name.endswith(".asm") and name.startswith(("th0", "th02_")):
        return "rec98-asm"
    if name.casefold() == "c0.asm":
        return "borland-startup"
    return "borland-library"


def detailed_map(path: Path) -> list[dict[str, object]]:
    text = path.read_text(encoding="latin-1")
    section = text.split("Detailed map of segments", 1)[1].split("Address", 1)[0]
    rows = []
    for line in section.splitlines():
        match = LINE.match(line)
        if not match:
            continue
        segment, offset, length, klass, seg_name, group, module, acbp = match.groups()
        rows.append(
            {
                "start": int(segment, 16) * 16 + int(offset, 16),
                "length": int(length, 16),
                "class": klass,
                "segment": seg_name,
                "group": group,
                "module": module.replace("\\", "/"),
                "acbp": acbp,
            }
        )
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="cold ReC98 source directory (contains bin/ and obj/)")
    parser.add_argument("--output", type=Path, default=LEDGER)
    args = parser.parse_args()

    manifest = load_target_manifest(ROOT / "config" / "targets.toml")
    out_rows: list[dict[str, object]] = []
    summary: dict[str, dict[str, list[int]]] = {}

    for product in PRODUCTS:
        target = parse_mz(read_verified_artifact(ROOT, find_artifact(manifest, f"th02-{product}"))).program_image
        built = parse_mz((args.source / "bin" / "th02" / f"{product}.exe").read_bytes()).program_image
        if len(target) != len(built):
            print(f"{product}: program sizes differ ({len(target)} != {len(built)}); layout not comparable", file=sys.stderr)
            return 1
        rows = detailed_map(args.source / "obj" / "th02" / f"{product}.map")
        totals = summary.setdefault(product, {})
        covered = bytearray(len(target))
        for order, row in enumerate(rows):
            start, length = int(row["start"]), int(row["length"])
            initialized = row["class"] in INITIALIZED
            differing = 0
            if initialized and length:
                end = min(start + length, len(target))
                differing = sum(1 for i in range(start, end) if target[i] != built[i])
                for i in range(start, end):
                    covered[i] = 1
            if not length:
                state = "empty"
            elif not initialized:
                state = "uninitialized"
            elif differing:
                state = "differs"
            else:
                state = "byte-equal"
            kind = kind_of(str(row["module"]))
            if kind == "rec98-asm" and row["segment"] == "_TEXT":
                # The asm files' _TEXT is dominated by included master.lib
                # and other library sources; keep it apart from game segments.
                kind = "rec98-asm-text"
            out_rows.append(
                {
                    "product": product.upper() + ".EXE",
                    "order": order,
                    "module": row["module"],
                    "kind": kind,
                    "class": row["class"],
                    "segment": row["segment"],
                    "start": f"{start:05X}",
                    "length": length,
                    "differing_bytes": differing,
                    "state": state,
                }
            )
            if initialized and length:
                bucket = totals.setdefault(f"{kind}/{row['class']}", [0, 0, 0])
                bucket[0] += length
                bucket[1] += length if not differing else 0
                bucket[2] += differing
        gaps = [i for i in range(len(target)) if not covered[i]]
        gap_diffs = [i for i in gaps if target[i] != built[i]]
        totals["linker-gap"] = [len(gaps), len(gaps) - len(gap_diffs), len(gap_diffs)]
        if gap_diffs:
            out_rows.append(
                {
                    "product": product.upper() + ".EXE",
                    "order": len(rows),
                    "module": "(alignment gaps)",
                    "kind": "linker-gap",
                    "class": "-",
                    "segment": "-",
                    "start": " ".join(f"{i:05X}" for i in gap_diffs),
                    "length": len(gaps),
                    "differing_bytes": len(gap_diffs),
                    "state": "differs",
                }
            )

    zun_target = find_artifact(manifest, "th02-zun")
    zun_built = args.source / "bin" / "th02" / "zun.com"
    zun_equal = digest_file(zun_built) == zun_target["sha256"]
    out_rows.append(
        {
            "product": "ZUN.COM",
            "order": 0,
            "module": "(whole file: ONGCHK, ZUNINIT, ZUN_RES, ZUNSOFT)",
            "kind": "rec98-zungen",
            "class": "-",
            "segment": "-",
            "start": "00000",
            "length": zun_built.stat().st_size,
            "differing_bytes": 0 if zun_equal else -1,
            "state": "byte-equal" if zun_equal else "differs",
        }
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(out_rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(out_rows)

    for product, totals in summary.items():
        print(f"{product.upper()}.EXE")
        for key in sorted(totals):
            total, equal_bytes, differing = totals[key]
            print(f"  {key:32} {total:7d} B  in byte-equal contributions {equal_bytes:7d}  differing bytes {differing}")
    print(f"ZUN.COM whole file byte-equal: {zun_equal}")
    print(f"ledger: {args.output.relative_to(ROOT) if args.output.is_relative_to(ROOT) else args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
