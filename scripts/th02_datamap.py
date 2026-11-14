#!/usr/bin/env python3
"""DGROUP offset -> th02_main.asm data line, via a fresh TASM listing + map.

    python3 scripts/th02_datamap.py 91FC 26CE6   # hex DGROUP offsets

Use it for raw addresses such as [bx-6E04h] (= 91FCh) before labelling a
`dup(?)` block so that C++ can name the data.
"""
import os, re, subprocess, sys

WORK = os.path.expanduser("~/coding/codex_ida/th02-reconstruction/th02/.analysis/work/rec98")


def listing():
    env = dict(os.environ, WINEDEBUG="-all",
               PATH=os.path.expanduser("~/coding/codex_ida/th02-reconstruction/th02/.tools/factory-bin") + ":" + os.environ["PATH"])
    subprocess.run(["wine", r"C:\TASM50\bin\TASM32.EXE", "/m", "/mx", "/kh32768", "/t", "/dGAME=2", "/la",
                    "th02_main.asm", r"obj\th02\lst.obj", r"obj\th02\main.lst"], cwd=WORK, env=env,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    return open(f"{WORK}/obj/th02/main.lst", encoding="latin-1").read().splitlines()


def bases():
    m = open(f"{WORK}/obj/th02/main.map", encoding="latin-1").read()
    out = {}
    dgroup = None
    for seg in ("_DATA", "_BSS"):
        r = re.search(r"([0-9A-F]{4}):([0-9A-F]{4}) [0-9A-F]{4,5} C=\S+\s+S=" + seg + r"\s+G=DGROUP\s+M=th02_main\.asm", m)
        out[seg] = int(r[2], 16)
    return out


def entries():
    """Yield (segment, dgroup_offset, prev_label, lines_after_label, text) for level-0 data lines."""
    base = bases()
    seg = None
    label, after = None, 0
    src = open(f"{WORK}/th02_main.asm", "rb").read().decode("latin-1").splitlines()
    # walk source and listing in parallel over level-0 lines
    lst = [l for l in listing() if re.match(r"^\s{0,1}\s*\d+\s", l) and not l[:1].isdigit()]
    for l in lst:
        m = re.match(r"^\s*(\d+)\s+([0-9A-F]{4})\s", l)
        text = l[40:] if len(l) > 40 else ""
        s = re.search(r"\b(\.data\??|_DATA\s+segment|_BSS\s+segment)", l)
        if ".data?" in l and re.search(r"^\S*\s*\d+\s+\.data\?", l.strip()) is None:
            pass
        if re.search(r"\t\s*\.data\?\s*$", l) or re.search(r"\t\s*\.data\?$", l):
            seg = "_BSS"; continue
        if re.search(r"\t\s*\.data\s*$", l):
            seg = "_DATA"; continue
        if re.search(r"\tsegment\b|\t\s*\S+\s+segment\b", l) and "_DATA" not in l and "_BSS" not in l:
            seg = None
        if seg and m:
            yield seg, base[seg] + int(m[2], 16), l


if __name__ == "__main__":
    want = [int(x, 16) for x in sys.argv[1:]]
    ents = list(entries())
    for w in want:
        best = None
        for e in ents:
            if e[1] <= w:
                best = e
        print(f"{w:04X}: {best[0]} {best[1]:04X} | {best[2].strip()[:100]}")
