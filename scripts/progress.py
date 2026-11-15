#!/usr/bin/env python3
"""Generate docs/PROGRESS.md and resources/progress.svg from the TH02 ledgers.

The only input is config/rec98_th02_units.csv (written by build_th02.py
--write-ledger). CODE bytes are split into:

* source  - C/C++ contributions (ReC98 or src/) byte-equal to the target
* differs - C/C++ contributions with differing bytes
* asm     - assembly modules in game segments (ZUN's own assembly in MAIN)
* library - Borland runtime, c0 and the asm files' _TEXT (master.lib etc.)
"""

from __future__ import annotations

import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LEDGER = ROOT / "config" / "rec98_th02_units.csv"
PRODUCTS = ("OP.EXE", "MAIN.EXE", "MAINE.EXE")
COLORS = {"source": "#55c7a6", "differs": "#f7768e", "asm": "#e0af68", "library": "#565f89"}
LABELS = {
    "source": "C/C++ byte-equal",
    "differs": "C/C++ differing",
    "asm": "ZUN assembly modules",
    "library": "runtime + asm _TEXT (mostly master.lib)",
}


def bucket(row: dict[str, str]) -> str | None:
    if row["class"] != "CODE":
        return None
    kind = row["kind"]
    if kind == "rec98-c":
        return "source" if row["state"] == "byte-equal" else "differs"
    if kind == "rec98-asm":
        return "asm"
    return "library"


def totals() -> tuple[dict[str, dict[str, int]], bool]:
    result = {product: dict.fromkeys(COLORS, 0) for product in PRODUCTS}
    zun = False
    with LEDGER.open(encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            if row["product"] == "ZUN.COM":
                zun = row["state"] == "byte-equal"
                continue
            key = bucket(row)
            if key:
                result[row["product"]][key] += int(row["length"])
    return result, zun


ZUN_ASM = ("th02/main/pointnum/num_put_m.asm", "th02/main/hittest.asm")


def authored() -> dict[str, dict[str, int]]:
    """Authored bytes per product: C/C++ CODE and DATA plus ZUN's own asm
    modules. master.lib, the Borland runtime, BSS and padding are excluded."""
    result = {p: {"c": 0, "asm": 0, "data": 0, "exact": 0, "total": 0, "tus": 0} for p in PRODUCTS}
    with LEDGER.open(encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            v = result.get(row["product"])
            n = int(row["length"])
            if v is None or not n or row["class"] not in ("CODE", "DATA"):
                continue
            if row["kind"] == "rec98-c":
                key = "c" if row["class"] == "CODE" else "data"
                v["tus"] += row["class"] == "CODE"
            elif row["module"].replace("\\", "/") in ZUN_ASM:
                key = "asm" if row["class"] == "CODE" else "data"
            else:
                continue
            exact = row["state"] == "byte-equal"
            v[key + "_exact"] = v.get(key + "_exact", 0) + (n if exact else 0)
            v[key] += n
            v["total"] += n
            v["exact"] += n if exact else 0
    return result


def svg(data: dict[str, dict[str, int]], zun: bool) -> str:
    del data
    auth = authored()
    width, left, right, wl, wr = 788, 200, 488, 250, 270
    text = 'font-family="sans-serif"'
    rows = []
    y = 137
    for product in PRODUCTS + ("ZUN.COM",):
        rows.append(f'<g aria-label="{product}: exact authored bytes; ASM is ZUN\'s original assembly">')
        rows.append(f'<text x="24" y="{y + 7}" fill="#f4f4f5" {text} font-size="16" font-weight="600">{product}</text>')
        if product != "ZUN.COM":
            rows.append(f'<rect x="{left}" y="{y}" width="{wl}" height="12" rx="4" fill="#353b52"/>')
        rows.append(f'<rect x="{right}" y="{y}" width="{wr}" height="12" rx="4" fill="#353b52"/>')
        if product == "ZUN.COM":
            rows.append(f'<text x="24" y="{y + 27}" fill="#c8cad2" {text} font-size="12">4 packed programs</text>')
            if zun:
                rows.append(f'<rect x="{right}" y="{y}" width="{wr}" height="12" fill="#55c7a6"/>')
            rows.append(f'<text x="{left}" y="{y + 10}" fill="#c8cad2" {text} font-size="12">ZUNINIT, ONGCHK ASM · 2 C/C++ programs</text>')
            rows.append(f'<text x="{right}" y="{y + 29}" fill="#c8cad2" {text} font-size="12">whole file {"byte-equal" if zun else "differs"}</text>')
        else:
            v = auth[product]
            code = v["c"] + v["asm"]
            rows.append(f'<text x="24" y="{y + 27}" fill="#c8cad2" {text} font-size="12">{v["tus"]} C/C++ TUs</text>')
            if code:
                rows.append(f'<rect x="{left}" y="{y}" width="{wl * v.get("c_exact", 0) / code:.2f}" height="5" fill="#9b6de3"/>')
                rows.append(f'<rect x="{left}" y="{y + 7}" width="{wl * (v.get("c_exact", 0) + v.get("asm_exact", 0)) / code:.2f}" height="5" fill="#e0af68"/>')
            if v["total"]:
                rows.append(f'<rect x="{right}" y="{y}" width="{wr * v["exact"] / v["total"]:.2f}" height="12" fill="#55c7a6"/>')
            share = 100 * v["exact"] / v["total"] if v["total"] else 0
            rows.append(f'<text x="{left}" y="{y + 29}" fill="#c8cad2" {text} font-size="12">C/C++ {v["c"]:,} B · ASM {v["asm"]:,} B</text>')
            rows.append(f'<text x="{right}" y="{y + 29}" fill="#c8cad2" {text} font-size="12">{share:.3f}% · {v["exact"]:,} / {v["total"]:,} B</text>')
        rows.append("</g>")
        y += 76
    exact = sum(v["exact"] for v in auth.values())
    total = sum(v["total"] for v in auth.values())
    y -= 15
    rows.append(f'<path d="M24 {y} H764" stroke="#353b52"/>')
    rows.append(f'<text x="24" y="{y + 28}" fill="#f4f4f5" {text} font-size="13" font-weight="600">'
                f'Exact authored bytes: {exact:,} / {total:,} B ({100 * exact / total:.3f}%)</text>')
    rows.append(f'<text x="24" y="{y + 56}" fill="#c8cad2" {text} font-size="12">Details: docs/PROGRESS.md · goal 100%</text>')
    height = y + 84
    return "\n".join([
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}" role="img" aria-label="TH02 exact authored reconstruction progress">',
        "  <title>TH02 reconstruction progress by artifact</title>",
        "  <desc>Exact authored bytes are C/C++ CODE and DATA plus ZUN's original ASM modules. master.lib, the Borland runtime, BSS and padding are excluded. Generated by scripts/progress.py.</desc>",
        f'  <rect x="0.5" y="0.5" width="{width - 1}" height="{height - 1}" rx="16" fill="#1f2335" stroke="#353b52"/>',
        f'  <text x="24" y="39" fill="#f4f4f5" {text} font-size="22" font-weight="600">TH02 reconstruction progress</text>',
        f'  <text x="24" y="64" fill="#c8cad2" {text} font-size="13">Authored bytes · C/C++ and ZUN\'s original ASM · master.lib, runtime, BSS and padding excluded</text>',
        f'  <text x="{left}" y="103" fill="#f4f4f5" {text} font-size="13" font-weight="600">Exact CODE · C/C++ / with ASM</text>',
        f'  <text x="{right}" y="103" fill="#f4f4f5" {text} font-size="13" font-weight="600">Exact authored bytes</text>',
        *("  " + line for line in rows),
        "</svg>",
        "",
    ])


def markdown(data: dict[str, dict[str, int]], zun: bool) -> str:
    lines = [
        "# Progress",
        "",
        "Generated by `scripts/progress.py` from `config/rec98_th02_units.csv`.",
        "Bytes are CODE-class linker-map contributions compared with the original",
        "Japanese executables. Library covers the Borland runtime, c0 and the",
        "assembly files' `_TEXT`: master.lib plus ZUN's hand-written `pfopen()`.",
        "ASM game code is MAIN's `shots_hittest()` and `pointnum_put()`. ZUN wrote",
        "both as assembly modules (TASM fixup order, a word-aligned code segment,",
        "`enter 0, 0`), so they stay assembly. All other ZUN code is C/C++.",
        "ZUN.COM packs ZUNINIT (ZUN's assembly), ONGCHK (KAJA's board probe,",
        "assembled from recovered source), ZUN_RES and ZUNSOFT (C/C++).",
        "",
        "| Product | C/C++ byte-equal | C/C++ differing | ASM game code | Library | Game segments in C/C++ |",
        "| --- | ---: | ---: | ---: | ---: | ---: |",
    ]
    for product in PRODUCTS:
        v = data[product]
        game = v["source"] + v["differs"] + v["asm"]
        share = 100 * v["source"] / game if game else 0
        lines.append(f"| {product} | {v['source']:,} | {v['differs']:,} | {v['asm']:,} | {v['library']:,} | {share:.1f}% |")
    lines += ["", f"ZUN.COM: whole file {'byte-equal' if zun else 'differs'}.", ""]
    return "\n".join(lines)


def main() -> int:
    data, zun = totals()
    (ROOT / "resources").mkdir(exist_ok=True)
    (ROOT / "resources" / "progress.svg").write_text(svg(data, zun), encoding="utf-8")
    (ROOT / "docs" / "PROGRESS.md").write_text(markdown(data, zun), encoding="utf-8")
    print((ROOT / "docs" / "PROGRESS.md").read_text(encoding="utf-8"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
