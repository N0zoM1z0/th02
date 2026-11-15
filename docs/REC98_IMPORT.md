# ReC98 TH02 import

[ReC98](https://github.com/nmlgc/ReC98) by nmlgc and contributors has already
decompiled most of TH02 outside MAIN's gameplay. This repository starts from
that work. It is pinned at `b6ba5b0a529edbb31efdf8c0e939263804f8ee47`, which
was upstream `master` on 2026-11-14 and is also the TH03 pin.

ReC98 has no top-level license, so it is not vendored. Builds materialize the
pinned tree with `git archive`, copy this repository's `src/` and
`compat/rec98/` files over it (`config/rec98_overlay.toml`), and run ReC98's
own `build.bat`. Only files we wrote or had to change live here.

## How the import was checked

1. `scripts/import_targets.py` extracted `HUMAROKU/` from the user's
   `jp.hdi`. All 20 TH01–TH05 programs match the hashes pinned by TH03/TH04.
2. `scripts/cold_build_rec98.py` cold-built unmodified ReC98. Every TH02
   output equals the hash TH03 recorded for the same revision
   (`config/rec98_pc98_calibration.toml`), so the toolchain is reproducible
   across repositories.
3. `scripts/rec98_th02_ledger.py` reads each product's TLINK detailed segment
   map and compares every module contribution's bytes with the original.
   Program images have identical sizes and identical relocation sets, so the
   comparison is positional. The result is `config/rec98_th02_units.csv`.

## Findings

**ZUN.COM** is byte-identical as a whole file, including ZUNINIT, ZUN_RES and
the ZUNGEN packing.

**Every ReC98 C/C++ unit in OP, MAIN and MAINE is byte-equal**, after one fix:

- `src/main/player/bomb.cpp`, `bomb_bft_8tiles_put_8()`: the original ends
  its loop with `cmp si, 8 / jl` (MAIN program offset 0xEEBB). Borland's
  `_SI` is unsigned, so `while(_SI < BYTE_DOTS)` emits `jb`.
  `static_cast<int>(_SI)` emits `jl`. Fixed in `src/main/player/bomb.cpp`,
  verified with a TCC 4.02 probe and a full rebuild.

The remaining differing bytes are all in ReC98's assembly files:

| Site (program offset) | Module | Original | ReC98 | Cause |
| --- | --- | --- | --- | --- |
| OP 0x2D9D, MAIN 0x3555 | asm `_TEXT`, before `SUPER_PUT` | `00` | `90` | Alignment fill after master.lib's `SUPER_PUT_RECT`; the original was zero-filled. ReC98 uses `db 0` for the same case after `gdc_outpw`. |
| MAIN 0x0E2D | asm `_TEXT`, before `GRCG_CIRCLE_X` | `00` | `90` | Same zero fill. |
| OP 0xDBA5, MAIN 0x1E2C9, MAINE 0xD6DB | asm `_DATA` | `90` | `00` | The opposite case: the original's data alignment is NOP-filled (assembler `EVEN`). |
| MAIN 0xB978 | `sub_B3DA` (MAIN_01_TEXT) | `30 C0` | `32 C0` | `xor al, al`, other direction bit. |
| MAIN 0xBE9E–0xBECE (12 B) | `sub_BCB1` (MAIN_01_TEXT) | `88 E0`, `08 EC` | `8A C4`, `0A E5` | `mov al, ah` / `or ah, ch`, other direction bit. |

The MAIN_01_TEXT encodings are not an assembler-version issue. TASM32 5.0 and
DOS TASM 4.1 both emit `32 C0 / 8A C4 / 0A E5`. Turbo C++ 4.02's built-in
inline assembler emits `30 C0 / 88 E0 / 08 EC`, exactly the original, while
codegen for `_AL = _AH` emits `8A C4`. So ZUN wrote `sub_B3DA` and `sub_BCB1`
as C/C++ functions with inline `asm`. Decompiling them that way fixes these
bytes naturally; patching `db` bytes into the ASM would not be natural source.

### Status after the local fixes

The four alignment-fill rows are fixed by `db 0` before the `GRCG_CIRCLE_X`
and `SUPER_PUT` includes and by inlining `vs[data].asm` with its original
`90h` fill (TH02 files only; the include is shared with TH03–TH05). OP.EXE and
MAINE.EXE program images are now byte-identical. The 13 inline-asm bytes in
MAIN went away once `sub_B3DA` (stage init) and `sub_BCB1` (stage loop) were
decompiled with ZUN's inline `asm`; all three program images now match.

One more ReC98 prototype inaccuracy surfaced while decompiling callers:
`bullets_add_16x16()` takes `patnum` as `int`, not the byte-sized
`main_patnum_t`. Only `int, int` constant pairs are pushed as one
`push imm32`, as the original callers do.

### Container differences (solved)

At import time the EXE files still differed outside the program images:

- Header size: the originals have 256/416/139 header paragraphs (OP/MAIN/MAINE)
  against TLINK's 224/384/224. TLINK sizes the header from its far-fixup
  count *before* shortening same-segment far calls, so the original asm
  called its procedures with `nosmart` far calls (`nopcall` in `ReC98.inc`).
  MAINE's minimal header (relocations at 0x1C) comes from a post-link rewrite,
  reproduced by `src/maine/exemin.c`.
- Relocation order: TLINK emits fixups module by module in link order (TCC
  objects highest address first per 0x400-byte chunk, TASM objects
  ascending), so matching the order recovered ZUN's translation-unit
  partition and link order.

All four files are now byte-identical; see [the handoff](RE_HANDOFF.md).

## Coverage

See [PROGRESS.md](PROGRESS.md) for the generated numbers. All of ZUN's
compiled code and data in OP, MAIN and MAINE is C/C++. ZUN's own assembly is
`pointnum_put()`, `shots_hittest()`, TH02's modified master.lib `pfopen()` and
ZUN.COM's ZUNINIT (`src/zun/zuninit.asm`). ZUN.COM's ONGCHK (KAJA's PMD board
probe) is assembled from `src/zun/ongchk.asm`, recovered in our TH03
reconstruction; ZUN_RES and ZUNSOFT are ReC98 C++.
